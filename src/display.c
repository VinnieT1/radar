#include <zephyr/drivers/display.h>
#include <zephyr/device.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/zbus/zbus.h>
#include "display.h"
// #include <lvgl.h>

#define DT_DRV_COMPAT zephyr_dummy_dc

ZBUS_CHAN_DEFINE(chan_display_data, struct msg_display_data, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(0));

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_display_data);

ZBUS_CHAN_ADD_OBS(chan_display_data, msub_display_data, 3);

int display_api_send(double velocity, bool is_over_limit, k_timeout_t timeout)
{
	struct msg_display_data msg = {.velocity = velocity, .is_over_limit = is_over_limit};

	return zbus_chan_pub(&chan_display_data, &msg, timeout);
}

void display_thread(void *ptr1, void *ptr2, void *ptr3)
{
	ARG_UNUSED(ptr1);
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);

	int err;
	char display_text[32];
	uint8_t rounded_velocity = 0;
	// lv_obj_t *label = lv_label_create(lv_scr_act());

	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	struct display_capabilities caps;
	uint8_t bpp;

	display_blanking_off(display_dev);
	display_get_capabilities(display_dev, &caps);

	switch (caps.current_pixel_format) {
	case PIXEL_FORMAT_RGB_565:
		bpp = 2;
		break;
	case PIXEL_FORMAT_RGB_888:
		bpp = 3;
		break;
	case PIXEL_FORMAT_MONO10:
	case PIXEL_FORMAT_MONO01:
		bpp = 1;
		break;
	case PIXEL_FORMAT_ARGB_8888:
		bpp = 4;
		break;
	default:
		bpp = 2;
		break;
	}

	const struct display_buffer_descriptor desc = {
		.width = caps.x_resolution,
		.height = caps.y_resolution,
		.pitch = caps.x_resolution,
		.buf_size = caps.x_resolution * caps.y_resolution * bpp,
	};

	const struct zbus_channel *chan;
	struct msg_display_data msg;

	while (1) {
		err = zbus_sub_wait_msg(&msub_display_data, &chan, &msg, K_FOREVER);
		if (err) {
			printf("error on display_thread sub_wait_msg...................\n");
			continue;
		}

		rounded_velocity = (msg.velocity > CONFIG_RADAR_SPEED_LIMIT_KMH &&
				    msg.velocity < CONFIG_RADAR_SPEED_LIMIT_KMH + 1)
					   ? (uint8_t)msg.velocity + 1
					   : (uint8_t)msg.velocity;

		snprintf(display_text, sizeof(display_text), "\t%d %s", rounded_velocity,
			 msg.is_over_limit ? "OVER LIMIT" : "OK");

		// lv_label_set_text(label, display_text);
		// lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
		// lv_timer_handler();

		printf("\t-~= DISPLAY =~\n%s\n", display_text);
		display_write(display_dev, 0, 0, &desc, display_text);
	}
}

K_THREAD_DEFINE(display_thread_id, 2048, display_thread, NULL, NULL, NULL, 3, 0, 0);