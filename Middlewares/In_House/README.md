# W5500 Ethernet Driver Stack for STM32F103

This directory contains a modular driver stack for the WIZnet W5500 Ethernet controller with specialized protocol support for STM32F103 microcontrollers. The codebase follows a layered architecture to maintain clean separation of concerns between hardware interfaces, networking APIs, and protocol-specific implementations.

## Quick Start

1. **Hardware Initialization**
   ```c
   #include "w5500.h"
   
   // Initialize W5500
   uint8_t mac[6] = {0x00, 0x08, 0xDC, 0xAB, 0xCD, 0xEF};
   uint8_t ip[4] = {192, 168, 1, 100};
   uint8_t subnet[4] = {255, 255, 255, 0};
   uint8_t gateway[4] = {192, 168, 1, 1};
   
   bool success = w5500_init(&hspi2, mac, ip, subnet, gateway);
   ```

2. **Socket Operations**
   ```c
   #include "w5500_socket.h"
   
   // Create and use a TCP socket
   uint8_t socket = w5500_socket(0, Sn_MR_TCP, 5000, 0);
   w5500_connect(socket, server_ip, server_port);
   ```

3. **ICMP Ping**
   ```c
   #include "icmp_ping.h"
   
   // Send ping
   icmp_ping_init(7);
   int32_t rtt = icmp_ping_send(target_ip, 32, 64, 1000);
   ```

## Architecture Overview

The codebase is organized into three main directories:

```
In_House/
├── custom_drivers/      # Hardware abstraction and socket API wrappers
├── custom_protocols/    # Protocol implementations (ICMP, micro-ROS)
└── custom_transports/   # Transport layer for micro-ROS
```

### Module Dependencies

![Architecture Diagram](https://i.imgur.com/d2guJ8X.png)

The architecture follows a layered approach with the following dependency flow:
1. Protocol implementations depend on socket wrappers
2. Socket wrappers depend on third-party socket API
3. Hardware initialization provides the foundation for all modules

## Module Descriptions

### custom_drivers

#### w5500.c/h

Low-level hardware abstraction layer for the W5500 Ethernet controller, providing:
- Hardware initialization and reset
- SPI communication interface
- Network configuration (IP, gateway, subnet, MAC)
- PHY status monitoring
- Utility functions (checksum calculation)

**Dependencies:**
- STM32 HAL (stm32f1xx_hal.h)
- WIZnet driver (wizchip_conf.h)

#### w5500_socket.c/h

Socket API wrapper around the third-party socket library, providing:
- Abstracted socket operations (create, connect, send, receive)
- Debug logging capabilities
- Error handling
- Const-correct interfaces

**Dependencies:**
- Third-party socket API (socket.h)
- w5500.h (for hardware-level functions)

### custom_protocols

#### icmp_ping.c/h

ICMP ping implementation with statistics tracking, providing:
- ICMP echo request/reply handling
- Ping functionality with timeouts
- Response time measurement
- Statistics collection (packets sent/received, round-trip times)

**Dependencies:**
- w5500_socket.h (for socket operations)

#### uros_publisher.c/h & uros_subscriber.c/h

micro-ROS publisher and subscriber implementations, providing:
- ROS topic publishing
- ROS topic subscription
- Message type handling

**Dependencies:**
- uros_transport.h (for micro-ROS communication)
- micro-ROS middleware libraries

### custom_transports

#### uros_transport.c/h

micro-ROS transport implementation using W5500 Ethernet, providing:
- Custom transport interface for micro-ROS
- Connection management to micro-ROS agent
- Timeout handling and reconnection logic

**Dependencies:**
- w5500_socket.h (for network communication)
- micro-ROS client libraries (uxr/client/transport.h, rmw_microros/rmw_microros.h)

## Usage Guidelines

### Hardware Initialization

Always initialize the W5500 hardware first using `w5500_init()` before using any socket or protocol functions:

```c
bool success = w5500_init(&hspi2, mac_addr, ip_addr, subnet_mask, gateway_addr);
if (!success) {
    // Handle initialization error
}
```

### Socket Operations

Use the socket wrapper functions instead of directly calling third-party socket functions:

```c
// Create a TCP socket
int8_t socket = w5500_socket(0, Sn_MR_TCP, 5000, 0);

// Connect to a server
w5500_connect(socket, server_ip, server_port);

// Send data
w5500_send(socket, data_buffer, data_length);
```

### ICMP Ping

Use the ICMP ping module for network testing and monitoring:

```c
// Initialize ping on socket 7
icmp_ping_init(7);

// Send ping to target IP
int32_t rtt = icmp_ping_send(target_ip, 32, 64, 1000);
if (rtt > 0) {
    printf("Ping successful, RTT: %ld ms\n", rtt);
}

// Get ping statistics
uint32_t sent, received, avg, min, max;
icmp_get_stats(&sent, &received, &avg, &min, &max);
```

### micro-ROS Communication

Initialize the micro-ROS transport before creating publishers/subscribers:

```c
// Initialize micro-ROS transport
uros_transport_init(agent_ip, agent_port);

// Get transport parameters for micro-ROS
rmw_uros_transport_params_t params = uros_get_transport();

// Set custom transport
rmw_uros_set_custom_transport(
    params.framing,
    params.args,
    params.open,
    params.close,
    params.write,
    params.read
);
```

## Design Considerations

1. **Modularity**: Each component has a single responsibility and clear interfaces
2. **Abstraction**: Hardware details are abstracted away from protocol implementations
3. **Error handling**: All functions include proper error checking and reporting
4. **Debug support**: Configurable debug output for development and troubleshooting
5. **Documentation**: Comprehensive function documentation in headers

## Coding Standards

This codebase follows these coding standards:

1. **Function Naming**: `module_function_name()` format for clarity
2. **Variable Naming**: Descriptive lowercase with underscores
3. **Constants**: All uppercase with underscores
4. **Header Guards**: In the format `_MODULE_H_`
5. **Documentation**: Doxygen-compatible comments for all public functions
6. **Error Codes**: Consistent return values across modules
7. **Const Correctness**: Input parameters marked as const where appropriate
8. **Type Safety**: Proper use of stdint.h types for portability

## Performance Notes

- The W5500 controller supports up to 8 simultaneous sockets (0-7)
- Socket 7 is typically reserved for ICMP ping functionality
- The micro-ROS transport uses one dedicated socket for communication with the agent
- Buffer sizes can be configured in w5500_init() to optimize memory usage

## Troubleshooting

### Common Issues

1. **No Network Connectivity**
   - Check physical connections (cable, power)
   - Verify IP configuration matches your network
   - Ensure MAC address is unique
   - Check link status with `w5500_get_link_status()`

2. **Socket Communication Failures**
   - Check socket status with `w5500_socket_status()`
   - Verify remote server is reachable with `icmp_ping_send()`
   - Ensure port numbers aren't blocked by firewalls
   - Check for correct byte ordering in IP addresses and ports

3. **micro-ROS Communication Issues**
   - Verify agent IP and port are correct
   - Check agent is running and accessible
   - Enable debug prints in uros_transport.c
   - Verify network configuration allows UDP traffic

### Debug Tips

- Enable debug printing by defining `W5500_DEBUG` in your build
- Use HAL_Delay() between operations to identify timing issues
- Monitor socket states during connections and transmissions
- Use ICMP ping to verify basic connectivity before troubleshooting higher-level protocols

## Implementation Notes

### Socket Buffer Allocation

The W5500 has 32KB of memory that is divided among the 8 available sockets. Default allocation is 2KB for TX and 2KB for RX per socket, but this can be adjusted based on your application's needs. Sockets that need to handle larger packets should be allocated more buffer space.

### Thread Safety

This driver implementation is not thread-safe by default. If used in a multi-threaded environment, proper synchronization mechanisms (mutexes, semaphores) should be implemented around socket and hardware access functions.

### Interrupt Handling

The current implementation uses polling rather than interrupts. For better performance in interrupt-driven applications, consider implementing an ISR for the W5500's interrupt pin and handling socket events asynchronously.

## Third-Party Dependencies

### WIZnet ioLibrary

This driver stack relies on WIZnet's ioLibrary for the core socket implementation. The specific version used is v3.2.0, located at:
```
Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/
```

Key components used:
- `socket.h` - Berkeley-like socket API
- `wizchip_conf.h` - W5500 configuration interface

### STM32 HAL

The hardware abstraction is built on STM32 HAL drivers, specifically:
- SPI interface for W5500 communication
- GPIO for chip select and reset control
- SYSTICK for timing functions

### micro-ROS (Optional)

If using the micro-ROS components, the following dependencies are required:
- rmw_microxrcedds
- uxr/client
- rmw_microros

These should be configured according to the micro-ROS getting started guide.
