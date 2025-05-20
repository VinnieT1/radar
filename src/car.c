#include <zephyr/random/random.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

static struct sensor_fixture {
	const struct gpio_dt_spec sensor_gpio;
} fixture1 = {
	.sensor_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(sensor1), gpios),
};

static struct sensor_fixture fixture2 = {
    .sensor_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(sensor2), gpios),
};

#define SENSOR_PRESS(_fixture) gpio_emul_input_set(_fixture.sensor_gpio.port, _fixture.sensor_gpio.pin, 0)

#define SENSOR_RELEASE(_fixture) gpio_emul_input_set(_fixture.sensor_gpio.port, _fixture.sensor_gpio.pin, 1)

void car_thread(void *ptr1, void *ptr2, void *ptr3)
{
	ARG_UNUSED(ptr1);
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);


    gpio_emul_input_set(fixture1.sensor_gpio.port, fixture1.sensor_gpio.pin, 1);
    gpio_emul_input_set(fixture2.sensor_gpio.port, fixture2.sensor_gpio.pin, 1);
	printk("Car starting...[ok]\n");

	while (1) {
        printk("Vroom... (Sensor 1)\n");
        
        SENSOR_PRESS(fixture1);
        SENSOR_RELEASE(fixture1);

        k_msleep(sys_rand8_get() % 20 + 21); // Random delay between 21 and 40 ms

        printk("Vroom... (Sensor 2)\n");
        SENSOR_PRESS(fixture2);
        SENSOR_RELEASE(fixture2);

        k_msleep(5000);
	}
}

K_THREAD_DEFINE(car_thread_id, 2048, car_thread, NULL, NULL, NULL, 3, 0, 0);