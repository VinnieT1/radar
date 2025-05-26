#include <stdbool.h>
#include <zephyr/zbus/zbus.h>
#include <stdio.h>
#include "sensor.h"
#include "camera_service.h"
#include "display.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_sensor_evt);

ZBUS_CHAN_ADD_OBS(chan_sensor_evt, msub_sensor_evt, 3);

void velocity_processing_thread(void *ptr1, void *ptr2, void *ptr3)
{
	ARG_UNUSED(ptr1);
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);

	int err;
	bool has_received_first = false;

	int64_t time_ref = 0;
	int64_t dt = 0;
	double velocity = 0;
	bool is_over_limit = false;

	struct msg_sensor_evt msg;
	const struct zbus_channel *chan;

	while (1) {
		err = zbus_sub_wait_msg(&msub_sensor_evt, &chan, &msg, K_FOREVER);
		if (err) {
			continue;
		}

		switch (msg.evt) {
		case SENSOR_EVT_DETECTED_FIRST:
			if (has_received_first) {
				printf("Sensor 1 detected again before sensor 2!\n");
			} else {
				time_ref = k_uptime_get();
			}

			has_received_first = true;

			break;
		case SENSOR_EVT_DETECTED_SECOND:
			if (has_received_first) {
				dt = k_uptime_get() - time_ref;
				velocity = 3.6 * CONFIG_RADAR_SENSOR_DISTANCE_MM / dt;

				printf("%d / %lld mm/ms\n", CONFIG_RADAR_SENSOR_DISTANCE_MM, dt);

				is_over_limit = (velocity > CONFIG_RADAR_SPEED_LIMIT_KMH);

				if (is_over_limit) {
					printf("Too fast! %lf km/h, triggering camera...\n\n",
					       velocity);
					err = camera_api_capture(K_MSEC(5));
					if (err) {
						printf("camera api not successful: %d\n", err);
					}
				} else {
					printf("Speed is ok! %lf km/h\n\n", velocity);
				}

				has_received_first = false;

				display_api_send(velocity, is_over_limit, K_FOREVER);
			} else {
				printf("Sensor 2 detected without Sensor 1!\n");
			}

			break;
		case SENSOR_EVT_UNDEFINED:
			printf("Sensor event undefined!\n");
			break;
		}
	}
}

K_THREAD_DEFINE(velocity_processing_thread_id, 2048, velocity_processing_thread, NULL, NULL, NULL,
		3, 0, 0);