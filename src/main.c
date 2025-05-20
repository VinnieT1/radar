#include <zephyr/sys/printk.h>
#include "camera_service.h"

int main(void)
{
    printk("Hello World! %s\n", CONFIG_BOARD);

    return 0;
}