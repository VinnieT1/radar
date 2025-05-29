# ZRadar: an Emulated Electronic Radar with Zephyr RTOS in QEMU

![alt text](/assets/ZRadarBG.png "ZRadar Logo")

## Project Description

This project implements a simplified electronic radar system using Zephyr RTOS on the emulated `qemu_cortex_m3` platform. The system detects the passage and speed of simulated vehicles, checks for speed violations, and sends violation data to a cloud web service. It also displays information on the terminal.

## Instructions to Run in QEMU

1. Clone this repository:

   ```sh
   git clone https://github.com/VinnieT1/radar.git
   cd radar
   ```

2. Compile and run the project:

   ```sh
   west build -b qemu_cortex_m3 -p always
   west build -t run
   ```

## Configurable Options via Kconfig

The following options can be adjusted in `prj.conf` or via menuconfig:

* `CONFIG_RADAR_SENSOR_DISTANCE_MM`: Distance between sensors (mm).
* `CONFIG_RADAR_SPEED_LIMIT_KMH`: Allowed speed limit (km/h).
* `CONFIG_HTTP_SERVER_IP`: Local service IP.
* `CONFIG_HTTP_SERVER_PORT`: Local service Port.
* `CONFIG_NTP_SERVER`: NTP server address.

## Architecture and Module Operation

### Block Diagram

The Block Diagram of the architecture used in this project is represented by the image below:
![alt text](/assets/BlockDiagram.jpg "ZRadar Block Diagram")

As soon as a car activates the first and then the second sensor, the system calculates the speed of the vehicle based on the time difference between the two sensor activations. If the speed exceeds the configured limit, a violation is detected, and a picture is taken of the vehicle's license plate. The license plate is then validated and, if the plate was read correctly, the violation data (plate, hash and timestamp) is sent to a HTTP service. The timestamp value is acquired by an NTP server request. The system also updates a display with the car speed and violation status.

### Threads and Functions

* **Velocity Processing Thread:** Receives sensor events, calculates speed, detects violations, and triggers the camera.
* **Sensor Functions:** Initializes and configures GPIOs, publishes events on ZBUS on interrupts when sensors are activated.
* **Display Thread:** Updates the virtual display with current speed and violation status.
* **Camera Thread:** Simulates image capture and generates vehicle license plate with hash. May return invalid plates randomly (intended).
* **Picture Transceiver Thread:** Validates license plates and sends data to server via HTTP. Uses NTP to obtain date/time.

### Communication

* **ZBUS**: Used for communication between threads.
* **GPIOs**: Simulate the sensors.
