#include <zephyr/sys/printk.h>
#include <stdbool.h>
#include "camera_service.h"
#include "sensor.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_sensor_evt);

ZBUS_CHAN_ADD_OBS(chan_sensor_evt, msub_sensor_evt, 3);

int main(void)
{
    int err;
    bool has_received_first = false;

    int64_t first_time;
    int64_t second_time;

    printk("Hello World! %s\n", CONFIG_BOARD);

    sensor_init();
    sensor_enable_interrupts();
    printk("Sensor initialized and interrupts enabled.\n");

    while (1) {
        struct msg_sensor_evt msg;
        const struct zbus_channel *chan;

        err = zbus_sub_wait_msg(&msub_sensor_evt, &chan, &msg, K_FOREVER);
        if (err) {
            continue;
        }

        switch (msg.evt) {
            case SENSOR_EVT_DETECTED_FIRST:
                if (has_received_first) {
                    printk("Sensor 1 detected again before sensor 2!\n");
                } else {
                    first_time = k_uptime_get();
                }

                has_received_first = true;
                
                break;
            case SENSOR_EVT_DETECTED_SECOND:
                if (has_received_first) {
                    second_time = k_uptime_get();

                    if (CONFIG_SENSOR_DISTANCE / (second_time - first_time) > CONFIG_SPEED_LIMIT ) {
                        printk("Too fast! %lld m/s\n\n", CONFIG_SENSOR_DISTANCE / (second_time - first_time));
                    } else {
                        printk("Speed is ok! %lld m/s\n\n", CONFIG_SENSOR_DISTANCE / (second_time - first_time));
                    }

                    has_received_first = false;
                } else {
                    printk("Sensor 2 detected without Sensor 1!\n");
                }

                break;
            case SENSOR_EVT_UNDEFINED:
                printk("Sensor event undefined!\n");
                break;
        }
    }
    
    return 0;
}