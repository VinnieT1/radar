#include "camera_service.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_camera_evt);

ZBUS_CHAN_ADD_OBS(chan_camera_evt, msub_camera_evt, 3);

static void remove_spaces_from_plate(const char plate[], char result[])
{
	uint8_t k = 0;

	for (uint8_t i = 0; plate[i] != '\0'; i += 1) {
		if (plate[i] != ' ') {
			result[k] = plate[i];
			k += 1;
		}
	}

	result[k] = '\0';
}

static int is_valid_plate(const char plate[])
{
	char processed_plate[16];
	bool is_valid = false;

	remove_spaces_from_plate(plate, processed_plate);

	printf("Processed plate: %s\n", processed_plate);

	if (strlen(processed_plate) != 7) {
		return -EINVAL;
	}

	// ARGENTINA LLNNNLL, BRASIL LLLNLNN, PARAGUAI LLLLNNN, URUGUAI LLLNNNN, VENEZUELA LLNNNLL
	if (isupper(processed_plate[0]) && isupper(processed_plate[1]) &&
	    isdigit(processed_plate[2]) && isdigit(processed_plate[3]) &&
	    isdigit(processed_plate[4]) && isupper(processed_plate[5]) &&
	    isupper(processed_plate[6])) {
		is_valid = true;
	} else if (isupper(processed_plate[0]) && isupper(processed_plate[1]) &&
		   isupper(processed_plate[2]) && isdigit(processed_plate[3]) &&
		   isupper(processed_plate[4]) && isdigit(processed_plate[5]) &&
		   isdigit(processed_plate[6])) {
		is_valid = true;
	} else if (isupper(processed_plate[0]) && isupper(processed_plate[1]) &&
		   isupper(processed_plate[2]) && isupper(processed_plate[3]) &&
		   isdigit(processed_plate[4]) && isdigit(processed_plate[5]) &&
		   isdigit(processed_plate[6])) {
		is_valid = true;
	}

	return is_valid ? 0 : -EINVAL;
}

static int validate_and_send(const char plate[], const char hash[])
{
	int err;
	err = is_valid_plate(plate);

	if (err) {
		return err;
	}

	printf("TODO: SEND TO SERVER...\n");

	return 0;
}

void picture_transceiver_thread(void *ptr1, void *ptr2, void *ptr3)
{
	ARG_UNUSED(ptr1);
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);

	int err;
	struct msg_camera_evt msg;
	const struct zbus_channel *chan;

	while (1) {
		err = zbus_sub_wait_msg(&msub_camera_evt, &chan, &msg, K_FOREVER);
		if (err) {
			continue;
		}

		switch (msg.type) {
		case MSG_CAMERA_EVT_TYPE_DATA:
			printk("\tPlate: %s\n\tHash: %s\n", msg.captured_data->plate,
			       msg.captured_data->hash);

			err = validate_and_send(msg.captured_data->plate, msg.captured_data->hash);
			switch (err) {
			case 0:
				printf("Sent to server\n");
				break;
			case -EINVAL:
				printf("Invalid plate...\n");
				break;
			default:
				printf("Unexpected error: %d", err);
				break;
			}
			break;
		case MSG_CAMERA_EVT_TYPE_ERROR:
			printf("Camera error, trying again...");
			camera_api_capture(K_MSEC(5));
			break;
		default:
			break;
		}
	}
}

K_THREAD_DEFINE(picture_transceiver_thread_id, 2048, picture_transceiver_thread, NULL, NULL, NULL,
		3, 0, 0);