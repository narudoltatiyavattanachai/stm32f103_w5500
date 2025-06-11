# W5500 Ethernet Evaluation with STM32F103C8T6

## Overview

This project implements a robust Ethernet communication system using the W5500 Ethernet controller with the STM32F103C8T6 microcontroller. It features a complete TCP/IP stack, FreeRTOS integration, and micro-ROS support, making it an ideal solution for IoT and embedded systems requiring network connectivity.

The implementation includes a custom transport layer for micro-ROS, enabling seamless integration with ROS 2 ecosystems. This allows the STM32F103 to function as a ROS 2 node, capable of publishing and subscribing to topics over Ethernet.

## Key Features

### Hardware
- **STM32F103C8T6**: 72MHz ARM Cortex-M3 microcontroller with 64KB Flash, 20KB SRAM
- **W5500 Ethernet Controller**: Hardware TCP/IP stack with 8 independent sockets
- **SPI2 Interface**: High-speed communication between MCU and W5500
- **Integrated PHY**: Simplifies hardware design

### Software
- **FreeRTOS**: Real-time operating system for task management
- **CMSIS V2**: Standardized interface for Cortex-M processors
- **Hardware Abstraction Layer (HAL)**: STM32CubeMX generated code
- **ITM Console**: Debug console over SWD interface
- **USB DFU Support**: Firmware updates over USB

### Networking
- **Static & DHCP Support**: Flexible network configuration
- **TCP/UDP Protocols**: Full protocol support
- **Socket API**: Easy-to-use network programming interface
- **Custom Transport Layer**: For micro-ROS integration

### ROS 2 Integration
- **micro-ROS Support**: Full ROS 2 node capabilities
- **ROS 2 Humble Compatible**: Tested with Humble Hawksbill
- **Custom W5500 Transport**: Optimized for embedded systems
- **Publisher/Subscriber Model**: Standard ROS 2 communication patterns

## Submodule Management

This project uses Git submodules to manage external dependencies. Here's how to work with them:

### Included Submodules

1. **ioLibrary_Driver_v3.2.0**
   - Version: v3.2.0
   - Purpose: Wiznet W5500 Ethernet controller driver
   - Location: `Middlewares/Third_Party/ioLibrary_Driver_v3.2.0`
   - Repository: [Wiznet/ioLibrary_Driver](https://github.com/Wiznet/ioLibrary_Driver)

2. **micro_ros_stm32cubemx_utils**
   - Version: 6.0.0 (compatible with ROS 2 Humble)
   - Purpose: Utilities for micro-ROS integration with STM32CubeMX
   - Location: `Middlewares/Third_Party/micro_ros_stm32cubemx_utils`
   - Repository: [micro-ROS/micro_ros_stm32cubemx_utils](https://github.com/micro-ROS/micro_ros_stm32cubemx_utils)

### Cloning the Repository with Submodules

```bash
git clone --recursive https://github.com/yourusername/stm32f103_w5500.git
cd stm32f103_w5500
```

### Updating Submodules

To update all submodules to their latest committed state:

```bash
git submodule update --init --recursive
```

### Switching Submodule Versions

To switch to a specific version of a submodule:

```bash
# Navigate to the submodule directory
cd Middlewares/Third_Party/ioLibrary_Driver_v3.2.0

# Checkout the desired version (example)
git checkout v3.2.0

# Return to project root and commit the update
cd ../../..
git add Middlewares/Third_Party/ioLibrary_Driver_v3.2.0
git commit -m "Update ioLibrary_Driver to v3.2.0"
```

## Project Structure

```
stm32f103_w5500/
├── Core/                    # Core STM32 application code
│   ├── Inc/                 # Header files
│   └── Src/                 # Source files
│
├── Drivers/                # STM32 HAL drivers and CMSIS
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
│
├── Middlewares/            # Middleware components
│   ├── Third_Party/
│   │   ├── ioLibrary_Driver_v3.2.0/     # W5500 driver
│   │   └── micro_ros_stm32cubemx_utils/  # micro-ROS utilities
│   └── ST/
│       └── STM32_USB_Device_Library/     # USB device stack
│
├── USB_DEVICE/            # USB Device configuration
├── .mxproject              # STM32CubeMX project file
└── README.md               # This file
```

## Hardware Setup

### Components
- STM32F103C8T6 Blue Pill board
- W5500 Ethernet module
- USB-TTL converter (for debugging)
- ST-Link V2 programmer
- Ethernet cable

### Pin Connections

| STM32F103 | W5500 Module |
|-----------|--------------|
| 3.3V     | 3.3V         |
| GND      | GND          |
| PB13     | SCK          |
| PB14     | MISO         |
| PB15     | MOSI         |
| PB12     | SS           |
| PB0      | RST          |
| PB1      | INT          |


## Getting Started

### Prerequisites
- STM32CubeIDE or your preferred ARM development environment
- ST-Link Utility or OpenOCD for flashing
- Terminal emulator (PuTTY, Tera Term, or similar)
- Python 3.x (for additional tools)

### Building the Project

1. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/yourusername/stm32f103_w5500.git
   cd stm32f103_w5500
   ```

2. Open the project in STM32CubeIDE
   - Select `File > Import > Existing Projects into Workspace`
   - Navigate to the project directory and select it

3. Build the project
   - Right-click on the project in Project Explorer
   - Select `Build Project` or press `Ctrl+B`

### Flashing the Firmware

1. Connect your ST-Link programmer to the board
2. In STM32CubeIDE:
   - Right-click on the project
   - Select `Debug As > STM32 Cortex-M C/C++ Application`
   - The debug configuration will be automatically created

### Network Configuration

By default, the device uses the following static IP configuration:
- IP: 192.168.68.200
- Netmask: 255.255.255.0
- Gateway: 192.168.68.1
- MAC: 00:08:DC:00:00:01

To change these settings, modify the `eth_config_init_static()` function in `Core/Src/eth_config.c`.

## Using micro-ROS

The project includes micro-ROS support with a custom W5500 transport layer. To use micro-ROS:

1. Set up a ROS 2 environment on your host machine
2. Start the micro-ROS agent:
   ```bash
   ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
   ```
3. The STM32 will automatically connect to the agent on startup

## Debugging

### ITM Console
For debugging output, connect an SWD debugger and use the ITM console:
1. Open a terminal emulator at 115200 baud
2. Connect to the ITM port (usually COMx in Windows)
3. Debug messages will be displayed in real-time

### LED Indicators
- **LED1 (PC13)**: Heartbeat (1Hz blink when running)
- **LED2 (PA1)**: Ethernet link status
- **LED3 (PA2)**: Network activity

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

```
│
├── Core/                    # Core STM32 application code
│   ├── Inc/                 # Header files
│   ├── Src/                 # Source files
│   └── Startup/             # Startup files for STM32F103
│
├── Drivers/                 # STM32 HAL and CMSIS drivers
│   ├── CMSIS/               # CMSIS V2 files
│   └── STM32F1xx_HAL_Driver/# STM32F1 HAL drivers
│
├── Middlewares/            # Middleware components
│   ├── In_House/          # Custom implementations
│   │   ├── custom_reponses/# Custom protocol handlers (e.g., ICMP)
│   │   └── custom_transports/ # Custom transport layers
│   │       └── w5500_transport.[c/h] # W5500 transport for micro-ROS
│   │
│   └── Third_Party/        # Third-party libraries
│       ├── FreeRTOS/       # FreeRTOS implementation
│       ├── ioLibrary_Driver/# W5500 driver library
│       │   ├── Ethernet/   # Ethernet implementation
│       │   ├── Internet/   # Internet protocols
│       │   └── Application/# Example applications
│       │
│       └── micro_ros_stm32cubemx_utils/ # micro-ROS utilities and examples
│           ├── extra_sources/           # Custom transport implementations
│           │   └── microros_transports/ # Transport layer implementations
│           ├── microros_static_library/ # Static library build system
│           └── sample_main_w5500.c      # Example micro-ROS application
│
├── docs/                   # Project documentation
│   └── modify_wizchip_conf.md # W5500 driver modifications
│
├── .gitignore              # Git ignore file
├── README.md               # Project readme (this file)
├── STM32F103C8TX_FLASH.ld  # Linker script
└── stm32f103_w5500.ioc    # STM32CubeMX project file
```

## Hardware Setup

### STM32F103C8T6 to W5500 Connections

| STM32F103 Pin | W5500 Pin | Description |
## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## References

- [W5500 Datasheet](https://www.wiznet.io/product-item/w5500/)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [CMSIS V2 Documentation](https://arm-software.github.io/CMSIS_5/General/html/index.html)
- [ioLibrary_Driver GitHub](https://github.com/Wiznet/ioLibrary_Driver)
- [micro-ROS Documentation](https://micro.ros.org/)
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
For detailed information about micro-ROS integration, please refer to the [micro-ROS documentation](https://micro.ros.org/).