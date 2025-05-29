#ifndef _SENSOR_H_
#define _SENSOR_H_
#include <zephyr/zbus/zbus.h>

enum sensor_evt_type {
	SENSOR_EVT_UNDEFINED,
	SENSOR_EVT_DETECTED_FIRST,
	SENSOR_EVT_DETECTED_SECOND,
};

struct msg_sensor_evt {
	enum sensor_evt_type evt;
};

ZBUS_CHAN_DECLARE(chan_sensor_evt);

/**
 * @brief Initialize the sensor GPIO pins.
 *
 * This function configures the GPIO pins for both sensors as input pins and
 * instantiates the SNTP context. It checks if the GPIO devices are ready and
 * configures them accordingly.
 *
 * @return 0 on success, negative error code on failure.
 */
int sensor_init(void);

/**
 * @brief Enable interrupts for the sensor GPIO pins.
 *
 * This function configures the GPIO pins for both sensor pins to trigger interrupts
 * on falling edges. It also initializes the callback function for handling sensor events.
 *
 * @return 0 on success, negative error code on failure.
 */
int sensor_enable_interrupts(void);

#endif /* _SENSOR_H_ */
