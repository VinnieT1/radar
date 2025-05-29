#include "sensor.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/sys/time_units.h>

ZBUS_CHAN_DEFINE(chan_sensor_evt, struct msg_sensor_evt, NULL, NULL, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.evt = SENSOR_EVT_UNDEFINED));

#define SENSOR1 DT_NODELABEL(sensor1)
#define SENSOR2 DT_NODELABEL(sensor2)

static const struct gpio_dt_spec sensor1 = GPIO_DT_SPEC_GET(SENSOR1, gpios);
static const struct gpio_dt_spec sensor2 = GPIO_DT_SPEC_GET(SENSOR2, gpios);

static struct gpio_callback sensor1_cb_data;
static struct gpio_callback sensor2_cb_data;

/**
 * @brief Sensor activated callback function.
 *
 * This function is called when the sensor is activated (e.g., falling edge).
 * It sends a message to the zbus channel with the event type.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param cb Pointer to the callback structure.
 * @param pins Bitmask of the pins that triggered the callback.
 */
void sensor_activated(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	struct msg_sensor_evt msg = {.evt = SENSOR_EVT_UNDEFINED};

	if (gpio_pin_get_dt(&sensor1)) {
		msg.evt = SENSOR_EVT_DETECTED_FIRST;

		printk("sensor1 activated at %lld\n", k_uptime_get());
	} else if (gpio_pin_get_dt(&sensor2)) {
		msg.evt = SENSOR_EVT_DETECTED_SECOND;

		printk("sensor2 activated at %lld\n", k_uptime_get());
	} else {
		return;
	}

	zbus_chan_pub(&chan_sensor_evt, &msg, K_NO_WAIT);
}

int sensor_init(void)
{
	int err;

	if (!gpio_is_ready_dt(&sensor1)) {
		printk("Error: sensor1 device %s is not ready\n", sensor1.port->name);
		return -ENODEV;
	}
	if (!gpio_is_ready_dt(&sensor2)) {
		printk("Error: sensor2 device %s is not ready\n", sensor2.port->name);
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&sensor1, GPIO_INPUT);
	if (err) {
		printk("Error %d: failed to configure %s pin %d\n", err, sensor1.port->name,
		       sensor1.pin);
		return err;
	}

	err = gpio_pin_configure_dt(&sensor2, GPIO_INPUT);
	if (err) {
		printk("Error %d: failed to configure %s pin %d\n", err, sensor2.port->name,
		       sensor2.pin);
		return err;
	}

	return 0;
}

int sensor_enable_interrupts(void)
{
	int err;

	err = gpio_pin_interrupt_configure_dt(&sensor1, GPIO_INT_EDGE_FALLING);
	if (err != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n", err,
		       sensor1.port->name, sensor1.pin);
		return err;
	}

	err = gpio_pin_interrupt_configure_dt(&sensor2, GPIO_INT_EDGE_FALLING);
	if (err != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n", err,
		       sensor2.port->name, sensor2.pin);
		return err;
	}

	gpio_init_callback(&sensor1_cb_data, sensor_activated, BIT(sensor1.pin));
	gpio_add_callback(sensor1.port, &sensor1_cb_data);

	gpio_init_callback(&sensor2_cb_data, sensor_activated, BIT(sensor2.pin));
	gpio_add_callback(sensor2.port, &sensor2_cb_data);

	return 0;
}