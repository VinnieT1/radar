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

#define SENSOR_PRESS(_fixture)                                                                     \
	gpio_emul_input_set(_fixture.sensor_gpio.port, _fixture.sensor_gpio.pin, 0)

#define SENSOR_RELEASE(_fixture)                                                                   \
	gpio_emul_input_set(_fixture.sensor_gpio.port, _fixture.sensor_gpio.pin, 1)

/**
 * @brief Simulates a car passing by the sensors in a loop.
 *
 * This function runs in a dedicated thread to simulate a car triggering
 * the sensors. It activates the first sensor, waits a randomized period
 * (simulating car travel time), then activates the second sensor. After
 * each complete pass, it waits 5 seconds before simulating another car.
 *
 * @param ptr1 Unused thread parameter
 * @param ptr2 Unused thread parameter
 * @param ptr3 Unused thread parameter
 */
void car_thread(void *ptr1, void *ptr2, void *ptr3)
{
	ARG_UNUSED(ptr1);
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);

	SENSOR_RELEASE(fixture1);
	SENSOR_RELEASE(fixture2);
	printk("Car starting...[ok]\n");

	SENSOR_PRESS(fixture1);
	SENSOR_RELEASE(fixture1);

	k_msleep(40); // 40ms

	printk("Vroom... (Sensor 2)\n");
	SENSOR_PRESS(fixture2);
	SENSOR_RELEASE(fixture2);

	k_msleep(5000);

	while (1) {
		printk("Vroom... (Sensor 1)\n");

		SENSOR_PRESS(fixture1);
		SENSOR_RELEASE(fixture1);

		k_msleep(sys_rand8_get() % 21 + 40); // Random delay between 40 and 60 ms

		printk("Vroom... (Sensor 2)\n");
		SENSOR_PRESS(fixture2);
		SENSOR_RELEASE(fixture2);

		k_msleep(5000);
	}
}

K_THREAD_DEFINE(car_thread_id, 2048, car_thread, NULL, NULL, NULL, 3, 0, 0);