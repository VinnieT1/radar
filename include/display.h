#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#include <stdbool.h>
#include <inttypes.h>

struct msg_display_data {
	double velocity;
	bool is_over_limit;
};

/**
 * @brief Publishes velocity data to the display channel.
 *
 * This function creates a display data message with the provided velocity and
 * over-limit status, then publishes it to the display channel using zbus.
 *
 * @param velocity The calculated vehicle velocity to display
 * @param is_over_limit Boolean indicating if the velocity exceeds the speed limit
 * @param timeout The maximum time to wait for publishing the message
 * @return 0 on success, <0 on failure
 */
int display_api_send(double velocity, bool is_over_limit, k_timeout_t timeout);

#endif /* _DISPLAY_H_ */