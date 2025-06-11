<div align="center">
  <h1>W5500 Ethernet Evaluation with STM32F103C8T6</h1>
</div>

## Next-Gen Edge Node for Smart Traffic Systems

This industrial-grade edge node solution revolutionizes traffic management by bringing powerful ROS 2 capabilities to the network edge. Built on the reliable STM32F103C8T6 microcontroller and W5500 Ethernet controller, it delivers real-time processing and decision-making where its cost matters most.

<div align="center">

  <img src="docs/pics/Demo_Board.png" alt="Demo Board" width="600" style="display: block; margin: 0 auto;">
</div>



### Why Choose Our Edge Node?

🚦 **Proven Reliability**
- Industrial-grade components ensure 24/7 operation in harsh environments
- Hardware-accelerated TCP/IP stack guarantees consistent communication
- Built-in watchdog and fail-safe mechanisms for maximum uptime

⚡ **Real-Time Performance**
- Near-zero latency decision making at the edge
- Deterministic response times with FreeRTOS
- Local processing reduces cloud dependency and bandwidth costs

🌐 **Seamless Integration**
- Native ROS 2 support for easy system integration
- Standard Ethernet connectivity for flexible deployment
- Compatible with existing smart city infrastructure

### Key Benefits

- **Cost-Effective**
  - Lower total cost of ownership with local processing
  - Reduced cloud computing and bandwidth expenses
  - Minimal maintenance requirements

- **Future-Proof**
  - Modular design for easy upgrades
  - Supports over-the-air (OTA) firmware updates
  - Scalable architecture for growing infrastructure needs

- **Deployment Ready**
  - Compact, low-power design
  - Wide operating temperature range
  - Easy installation and configuration

### Key Objectives

1. **Distributed Traffic Control**
   - Local processing at the edge for reduced latency
   - Decentralized decision making for improved reliability
   - Seamless integration with central traffic management systems

2. **Real-time Data Acquisition**
   - Interface with various traffic sensors (vehicle detection, pedestrian counters)
   - Environmental monitoring (air quality, weather conditions)
   - Traffic signal status and timing

3. **Communication Capabilities**
   - Reliable Ethernet-based communication using W5500
   - ROS 2 middleware for standardized messaging
   - Support for both wired and future wireless backhaul options
   - Secure communication channels for data integrity

4. **System Features**
   - Low-power operation for energy efficiency
   - Remote firmware updates (DFU over USB/Ethernet)
   - Local logging and diagnostics
   - Fail-safe operation with watchdog timers
   - Hot-swappable modules for easy maintenance

### System Architecture

```
┌───────────────────────────────────────────────────────────────────────┐
│                      Central Traffic Management                      │
│  ┌───────────────┐    ┌────────────────┐    ┌──────────────────┐  │
│  │  Cloud/Server │◄───┤  Edge Router    │◄───┤  Local Network   │  │
│  └───────────────┘    └────────────────┘    └──────────────────┘  │
└───────────────────────────────────────────────────────────▲──────────┘
                                                          │
                                                          │
┌──────────────────────────────────────────────────────────▼──────────┐
│                                                                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                     Smart Traffic Node                       │  │
│  │  ┌─────────────┐    ┌────────────────┐    ┌───────────────┐  │  │
│  │  │   Sensors   │    │  STM32F103     │    │  W5500        │  │  │
│  │  │ (Vehicle,   │◄───┤  + FreeRTOS    │◄───┤  Ethernet     │◄─┼──┘
│  │  │  Pedestrian)│    │  + micro-ROS   │    │  Controller   │  │
│  │  └─────────────┘    └────────────────┘    └───────────────┘  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
```

### Future Expansion

- Integration with V2X (Vehicle-to-Everything) communication
- Support for additional sensor types and protocols
- Edge AI capabilities for local traffic pattern analysis
- Solar power integration for remote deployment
- 5G/LoRaWAN connectivity options


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