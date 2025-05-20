#include <zephyr/sys/printk.h>
#include "camera_service.h"
#include "sensor.h"

int main(void)
{
    printk("Hello World! %s\n", CONFIG_BOARD);

    sensor_init();
    sensor_enable_interrupts();
    printk("Sensor initialized and interrupts enabled.\n");

    // TODO

    return 0;
}