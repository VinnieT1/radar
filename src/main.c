#include "sensor.h"
#include <stdio.h>
#include "display.h"

int main(void)
{
	int err;

	printf("Hello World! %s\n", CONFIG_BOARD);

	err = sensor_init();
	if (err) {
		printf("Error on sensor_init\n");
	}

	err = sensor_enable_interrupts();
	if (err) {
		printf("Error on sensor_enable_interrupts\n");
	}

	return 0;
}