#include "camera_service.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <zephyr/net/sntp.h>

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_camera_evt);

ZBUS_CHAN_ADD_OBS(chan_camera_evt, msub_camera_evt, 3);

/**
 * @brief Removes spaces from a license plate string.
 *
 * This function takes a license plate string and creates a new string
 * with all spaces removed.
 *
 * @param plate The input license plate string that may contain spaces
 * @param result The output buffer where the processed string will be stored
 */
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

/**
 * @brief Validates if a license plate follows supported format patterns.
 *
 * This function checks if a license plate follows one of the valid Mercosur
 * country formats (Argentina, Brazil, Paraguay, Uruguay, Venezuela).
 * The plate must be 7 characters after removing spaces.
 *
 * @param plate The license plate string to validate
 * @return 0 if the plate is valid, -EINVAL if invalid
 */
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

/**
 * @brief Validates a license plate and sends the data with timestamp.
 *
 * This function validates the license plate format, gets the current time
 * from an NTP server, adjusts it to Brazil time zone, and prepares the 
 * data for transmission to a server.
 *
 * @param plate The license plate string to process
 * @param hash The hash associated with the license plate
 * @return 0 on success, negative error code on failure
 */
static int validate_and_send(const char plate[], const char hash[])
{
	int err;
	struct sntp_time ts;
	char server_addr[64] = CONFIG_NTP_SERVER;
	time_t brazil_time;
	struct tm *brazil_tm;

	err = is_valid_plate(plate);
	if (err) {
		return err;
	}

	err = sntp_simple(server_addr, 500, &ts);
	if (err) {
		return err;
	}

	brazil_time = ts.seconds - (3 * 3600);
	brazil_tm = gmtime(&brazil_time);

	printk("Photo taken at: %04d-%02d-%02d %02d:%02d:%02d (Brazil time)\n",
		brazil_tm->tm_year + 1900, brazil_tm->tm_mon + 1, brazil_tm->tm_mday,
		brazil_tm->tm_hour, brazil_tm->tm_min, brazil_tm->tm_sec);


	// SEND TO PYTHON SERVER

	return 0;
}

/**
 * @brief Main thread for processing and transmitting camera events.
 *
 * This thread waits for camera events from the zbus channel, processes the 
 * captured license plate data, validates it, and sends it to the server.
 * It also handles camera errors by requesting a new capture.
 *
 * @param ptr1 Unused thread parameter
 * @param ptr2 Unused thread parameter
 * @param ptr3 Unused thread parameter
 */
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
				break;
			case -EINVAL:
				printf("Invalid plate...\n");
				break;
			case -ETIMEDOUT:
				printf("Server request timed out\n");
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