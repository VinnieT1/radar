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
 * This function is called when the sensor is activated (e.g., rising edge).
 * It sends a message to the zbus channel with the event type.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param cb Pointer to the callback structure.
 * @param pins Bitmask of the pins that triggered the callback.
 */
void sensor_activated(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    struct msg_sensor_evt msg = {.evt = SENSOR_EVT_UNDEFINED, /*.timestamp = {0}*/};

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

/**
 * @brief Initialize the sensor GPIO pins.
 *
 * This function configures the GPIO pins for both sensors as input pins and
 * instantiates the SNTP context. It checks if the GPIO devices are ready and
 * configures them accordingly.
 *
 * @return 0 on success, negative error code on failure.
 */
int sensor_init(void)
{
    int err;
    // struct sntp_time time_test;

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
        printk("Error %d: failed to configure %s pin %d\n", err, sensor1.port->name, sensor1.pin);
        return err;
    }

    err = gpio_pin_configure_dt(&sensor2, GPIO_INPUT);
    if (err) {
        printk("Error %d: failed to configure %s pin %d\n", err, sensor2.port->name, sensor2.pin);
        return err;
    }


    // err = sntp_simple(CONFIG_NTP_SERVER, 5000, &time_test);
    // if (err) {
    //     printk("SNTP request failed: %d", err);
        
    // } else {
    //     printk("Time from %s: %llu s, %u us", CONFIG_NTP_SERVER,
    //             time_test.seconds, time_test.fraction);
    // }

    return 0;
}

/**
 * @brief Enable interrupts for the sensor GPIO pins.
 *
 * This function configures the GPIO pins for both sensor pins to trigger interrupts
 * on falling edges. It also initializes the callback function for handling sensor events.
 *
 * @return 0 on success, negative error code on failure.
 */
int sensor_enable_interrupts(void)
{
    int ret;

    ret = gpio_pin_interrupt_configure_dt(&sensor1, GPIO_INT_EDGE_FALLING);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, sensor1.port->name, sensor1.pin);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&sensor2, GPIO_INT_EDGE_FALLING);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, sensor2.port->name, sensor2.pin);
        return ret;
    }

    gpio_init_callback(&sensor1_cb_data, sensor_activated, BIT(sensor1.pin));
    gpio_add_callback(sensor1.port, &sensor1_cb_data);

    gpio_init_callback(&sensor2_cb_data, sensor_activated, BIT(sensor2.pin));
    gpio_add_callback(sensor2.port, &sensor2_cb_data);

    return 0;
}