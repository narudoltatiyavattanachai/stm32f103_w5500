# W5500 Ethernet Evaluation with STM32F103C8T6 using CMSIS V2

## Overview

This project is designed to evaluate the W5500 Ethernet chip with the STM32F103C8T6 microcontroller using SPI2 interface and CMSIS V2. The W5500 is a hardwired TCP/IP embedded Ethernet controller that provides Internet connectivity for embedded systems using SPI communication. This evaluation platform demonstrates how to properly configure and use the W5500 with STM32F103C8T6 for robust Ethernet connectivity.

## Features

- **STM32F103C8T6 MCU**: ARM Cortex-M3 based 72MHz microcontroller
- **W5500 Ethernet Chip**: Hardware TCP/IP embedded Ethernet controller
- **SPI2 Interface**: Communication between STM32F103 and W5500
- **CMSIS V2**: Cortex Microcontroller Software Interface Standard V2
- **FreeRTOS**: Real-time operating system implementation
- **5 Sockets**: TCP/UDP socket communication capability
- **Hardware TCP/IP Stack**: Offloads TCP/IP processing from the MCU

## Project Structure

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
│   └── Third_Party/        # Third-party libraries
│       ├── FreeRTOS/       # FreeRTOS implementation
│       └── ioLibrary_Driver/# W5500 driver library
│           ├── Ethernet/   # Ethernet implementation
│           ├── Internet/   # Internet protocols
│           └── Application/# Example applications
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
|---------------|-----------|-------------|
| PB13 (SPI2_SCK)  | SCLK     | SPI Clock   |
| PB14 (SPI2_MISO) | MISO     | Master In Slave Out |
| PB15 (SPI2_MOSI) | MOSI     | Master Out Slave In |
| PB12          | SCSn     | Chip Select (Active Low) |
| 3.3V          | VCC      | Power Supply |
| GND           | GND      | Ground      |
| PA0 (Optional) | RSTn     | Reset (Active Low) |

## Software Implementation

### Driver Modifications

The W5500 driver library (ioLibrary_Driver v3.2.0) has been modified to work with the STM32F103C8T6 using SPI2. Key modifications include:

1. Implementation of chip select/deselect functions using PB12
2. SPI read/write functions using STM32 HAL SPI APIs
3. Integration with CMSIS V2 for better interrupt handling

### Configuration

- **SPI2 Configuration**: 18MHz clock, Master mode, 8-bit data size
- **W5500 Network Settings**: DHCP or static IP configuration supported
- **FreeRTOS**: Task-based approach for network operations
- **Socket Usage**: Multiple TCP/UDP sockets for different purposes

## Building and Running

### Prerequisites

- STM32CubeIDE or compatible IDE
- ST-Link V2 or compatible programmer
- W5500 Ethernet module
- Ethernet cable and network connection

### Build Steps

1. Clone this repository
2. Open the project in STM32CubeIDE
3. Build the project
4. Flash to the STM32F103C8T6 board
5. Connect the W5500 module to your network

### Testing

The evaluation project includes:

- Basic connectivity test
- Socket communication examples
- Performance measurements
- Reliability testing

## References

- [W5500 Datasheet](https://www.wiznet.io/product-item/w5500/)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [CMSIS V2 Documentation](https://arm-software.github.io/CMSIS_5/General/html/index.html)
- [ioLibrary_Driver GitHub](https://github.com/Wiznet/ioLibrary_Driver)

## License

This project is released under the MIT License.