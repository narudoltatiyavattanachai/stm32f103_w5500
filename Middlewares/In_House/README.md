
# WIZnet W5500 Custom Driver Implementation

## Overview

This directory contains custom driver implementations for the WIZnet W5500 Ethernet controller on STM32F103 microcontrollers. The drivers provide a modular, thread-safe interface to utilize all 8 hardware sockets of the W5500 for different network protocols and services.

## Socket Assignment Table

The W5500 provides 8 independent hardware sockets that are allocated to different protocols as follows:

| Socket | Module File | Service | Purpose |
|--------|-------------|---------|--------|
| 0 | w5500_dhcp.c | DHCP Client | IP Address Assignment |
| 1 | w5500_icmp.c | ICMP/Ping | Network Diagnostics |
| 2 | w5500_dns.c | DNS Client | Domain Name Resolution |
| 3 | w5500_tftp.c | TFTP Client | File Transfer |
| 4 | w5500_uros.c | micro-ROS Transport | ROS Communication |
| 5 | w5500_httpServer.c | Web Server (HTTP) | Configuration Interface |
| 6 | w5500_debug.c | Debug Console (CLI) | Debug Console |
| 7 | w5500_ota.c | OTA Firmware Update | Firmware Updates |

## Implementation Details

### Socket 0: DHCP Client
The DHCP client handles automatic IP address assignment for the device. It implements the DHCP protocol (discover, offer, request, acknowledgment) with retry logic and timeout handling.

### Socket 1: ICMP/Ping
Implements ICMP echo response (ping) functionality, allowing the device to respond to ping requests for network diagnostics and monitoring.

### Socket 2: Reserved
Reserved for future use.

### Socket 3: TFTP Client
Transfers files using the TFTP protocol. Primary use case is receiving binary files for OTA updates, which are stored in external SPI flash memory for later application.

### Socket 4: micro-ROS Transport
Implements a UDP-based transport layer for micro-ROS, enabling communication between the device and a micro-ROS agent. Features include:
- Automatic agent discovery
- Thread-safe socket operations with mutex protection
- Connection management with exponential backoff reconnection
- Heartbeat monitoring for connection health

### Socket 5: HTTP Server
Provides a web-based configuration interface with features including:
- Device information display
- System status monitoring
- Command manual access
- Firmware update initiation

### Socket 6: Reserved
Reserved for future use.

### Socket 7: OTA Update
Handles over-the-air firmware updates, working in conjunction with the TFTP client to apply firmware updates received via TFTP.

## Configuration

All socket-specific configuration parameters are centralized in `Core/Inc/ip_config.h`. This includes:
- Socket number assignments
- Port numbers
- Buffer sizes
- Timeout values
- Protocol-specific parameters

## Dependencies

- STM32F103 HAL Library
- CMSIS-RTOS v2 (for thread safety)
- WIZnet ioLibrary Driver v3.2.0 (Third Party)
- micro-ROS STM32CubeMX Utilities (for micro-ROS integration)

## Thread Safety

All socket operations are protected with CMSIS-RTOS v2 mutexes to ensure thread-safe operation in a multi-threaded RTOS environment.