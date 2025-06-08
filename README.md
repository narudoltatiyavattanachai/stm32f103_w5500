# STM32F103 + W5500 micro-ROS Ethernet Communication

## Overview

This project demonstrates how to implement Ethernet communication on an STM32F103 microcontroller using the W5500 Ethernet chip via SPI, integrating the LWIP TCP/IP stack and micro-ROS for ROS 2 communication. The project enables the STM32 to participate in ROS 2 networks over Ethernet, supporting micro-ROS publisher and subscriber services.

## Features

- **STM32F103 MCU**: ARM Cortex-M3 microcontroller as the main controller.
- **W5500 Ethernet Chip**: Hardware TCP/IP offload engine, interfaced via SPI.
- **LWIP Stack**: Lightweight IP stack for embedded Ethernet networking.
- **micro-ROS**: ROS 2 client library for microcontrollers, supporting pub/sub and services.
- **SPI Communication**: Efficient SPI protocol between STM32 and W5500.
- **ROS 2 Integration**: STM32 can publish and subscribe to ROS 2 topics and use ROS 2 services over Ethernet.

## Hardware Requirements

- STM32F103 development board (e.g., Blue Pill)
- W5500 Ethernet module
- SPI wiring between STM32 and W5500
- Ethernet cable and network access
- (Optional) ST-Link or compatible programmer/debugger

## Software Requirements

- STM32CubeMX / STM32CubeIDE (for configuration and code generation)
- HAL drivers for STM32F1
- LWIP middleware (integrated via CubeMX)
- micro-ROS library for STM32 (see [micro-ROS docs](https://micro.ros.org/docs/))
- FreeRTOS (optional, for RTOS-based projects)
- ROS 2 (Foxy/Humble or compatible) on your PC/robot

## Project Structure

- `Src/` and `Inc/`: Main source and header files for STM32 application
- `lwip/`: LWIP stack configuration and source
- `microros/`: micro-ROS client library and integration code
- `README.md`: Project documentation (this file)

## Setup & Usage

### 1. Hardware Connections

- Connect the STM32 SPI pins (SCK, MISO, MOSI, NSS) to the corresponding W5500 pins.
- Connect W5500 to your Ethernet network.

### 2. Firmware Configuration

- Use STM32CubeMX to enable SPI, LWIP, and (optionally) FreeRTOS.
- Configure LWIP for no OS or with FreeRTOS, depending on your project.
- Integrate the W5500 driver with LWIP (set as the netif driver).
- Add micro-ROS library and configure transport to use LWIP.

### 3. micro-ROS Pub/Sub Example

- Implement a micro-ROS publisher and subscriber in your main application.
- Example: Publish a sensor value to a ROS 2 topic, subscribe to a command topic.
- Ensure micro-ROS uses the LWIP transport layer.

### 4. Build & Flash

- Build the project in STM32CubeIDE.
- Flash the firmware to the STM32F103 board.

### 5. ROS 2 Integration

- On your ROS 2 PC, launch the micro-ROS agent:
  ```bash
  ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
  ```
- The STM32 will connect to the agent via Ethernet.
- Use standard ROS 2 tools (`ros2 topic echo`, `ros2 topic pub`) to interact with the STM32 node.

## References

- [micro-ROS Documentation](https://micro.ros.org/docs/)
- [W5500 Datasheet](https://www.wiznet.io/product-item/w5500/)
- [LWIP Documentation](https://www.nongnu.org/lwip/2_1_x/index.html)
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)

## License

This project is released under the MIT License.