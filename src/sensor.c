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


/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
// #define SW0_NODE DT_ALIAS(sw0)
// #if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
// #error "Unsupported board: sw0 devicetree alias is not defined"
// #endif
// static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
// static struct gpio_callback button_cb_data;

// static uint32_t button_press_time = 0;



/*
 * Get sensor configuration from the devicetree sensor0 and sensor1 aliases.
 */
#define SENSOR1 DT_NODELABEL(sensor1)
#define SENSOR2 DT_NODELABEL(sensor2)

// #if !DT_NODE_HAS_STATUS_OKAY(SENSOR0_NODE)
// #error "Unsupported board: sensor0 devicetree alias is not defined"
// #endif
// #if !DT_NODE_HAS_STATUS_OKAY(SENSOR1_NODE)
// #error "Unsupported board: sensor1 devicetree alias is not defined"
// #endif

static const struct gpio_dt_spec sensor0 = GPIO_DT_SPEC_GET(SENSOR1, gpios);
static const struct gpio_dt_spec sensor1 = GPIO_DT_SPEC_GET(SENSOR2, gpios);

static struct gpio_callback sensor0_cb_data;
static struct gpio_callback sensor1_cb_data;

static uint32_t sensor0_event_time = 0;
static uint32_t sensor1_event_time = 0;

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
    struct msg_sensor_evt msg = {.evt = SENSOR_EVT_UNDEFINED};

    if (gpio_pin_get_dt(&sensor0)) {
        msg.evt = SENSOR_EVT_DETECTED_FIRST;
        sensor0_event_time = k_uptime_get();

        printk("Sensor0 activated at %" PRIu32 "\n", sensor0_event_time);
    } else if (gpio_pin_get_dt(&sensor1)) {
        msg.evt = SENSOR_EVT_DETECTED_SECOND;
        sensor1_event_time = k_uptime_get();

        printk("Sensor1 activated at %" PRIu32 "\n", sensor1_event_time);
    } else {
        return;
    }

    zbus_chan_pub(&chan_sensor_evt, &msg, K_NO_WAIT);
}

/**
 * @brief Initialize the sensor GPIO pins.
 *
 * This function configures the GPIO pins for both sensors as input pins.
 *
 * @return 0 on success, negative error code on failure.
 */
int sensor_init(void)
{
    int ret;

    if (!gpio_is_ready_dt(&sensor0)) {
        printk("Error: sensor0 device %s is not ready\n", sensor0.port->name);
        return -ENODEV;
    }
    if (!gpio_is_ready_dt(&sensor1)) {
        printk("Error: sensor1 device %s is not ready\n", sensor1.port->name);
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&sensor0, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n", ret, sensor0.port->name, sensor0.pin);
        return ret;
    }

    ret = gpio_pin_configure_dt(&sensor1, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n", ret, sensor1.port->name, sensor1.pin);
        return ret;
    }

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

    ret = gpio_pin_interrupt_configure_dt(&sensor0, GPIO_INT_EDGE_FALLING);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, sensor0.port->name, sensor0.pin);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&sensor1, GPIO_INT_EDGE_FALLING);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, sensor1.port->name, sensor1.pin);
        return ret;
    }

    gpio_init_callback(&sensor0_cb_data, sensor_activated, BIT(sensor0.pin));
    gpio_add_callback(sensor0.port, &sensor0_cb_data);

    gpio_init_callback(&sensor1_cb_data, sensor_activated, BIT(sensor1.pin));
    gpio_add_callback(sensor1.port, &sensor1_cb_data);

    return 0;
}