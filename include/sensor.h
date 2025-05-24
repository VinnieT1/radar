#ifndef _SENSOR_H_
#define _SENSOR_H_
#include <zephyr/zbus/zbus.h>
// #include <zephyr/net/sntp.h>

enum sensor_evt_type {
	SENSOR_EVT_UNDEFINED,
	SENSOR_EVT_DETECTED_FIRST,
	SENSOR_EVT_DETECTED_SECOND,
};

struct msg_sensor_evt {
	enum sensor_evt_type evt;
	// struct sntp_time timestamp;
};

ZBUS_CHAN_DECLARE(chan_sensor_evt);

int sensor_init(void);
int sensor_enable_interrupts(void);

#endif /* _SENSOR_H_ */
