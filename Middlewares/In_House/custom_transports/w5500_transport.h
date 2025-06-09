/**
 * @file w5500_transport.h
 * @brief W5500 transport interface for regular usage and micro-ROS integration
 * @author Your Name
 * @date 2025-06-09
 */

#ifndef _W5500_TRANSPORT_H_
#define _W5500_TRANSPORT_H_

#include <stdbool.h>
#include <stdint.h>

/* Detect if we're building for micro-ROS or standard mode */
#ifdef RMW_UXRCE_TRANSPORT_CUSTOM
#include <uxr/client/transport.h>
#endif

/* Public function declarations ---------------------------------------------*/

/**
 * @brief Initialize the W5500 hardware
 * 
 * @param mac MAC address for the W5500 (6 bytes)
 * @param ip IP address for the local device (4 bytes)
 * @param subnet Subnet mask (4 bytes)
 * @param gateway Gateway IP address (4 bytes)
 * @return true if initialization successful, false otherwise
 */
bool w5500_init(const uint8_t mac[6], const uint8_t ip[4], 
                const uint8_t subnet[4], const uint8_t gateway[4]);

/**
 * @brief Open a socket for communication with the micro-ROS agent
 * 
 * @param agent_ip Agent IP address (4 bytes)
 * @param agent_port Agent port number
 * @param local_port Local port number
 * @return true if socket opened successfully, false otherwise
 */
bool w5500_socket_open(const uint8_t agent_ip[4], uint16_t agent_port, uint16_t local_port);

/**
 * @brief Close the socket
 * 
 * @return true if closed successfully, false otherwise
 */
bool w5500_socket_close(void);

/**
 * @brief Poll socket for incoming data with timeout
 * 
 * @param timeout_ms Timeout in milliseconds
 * @return true if data is available, false otherwise
 */
bool w5500_poll(uint32_t timeout_ms);

/**
 * @brief Get micro-ROS custom transport initialization functions
 * 
 * This sets up the custom transport for micro-ROS with the following functions:
 * - open: establishes connection with agent
 * - close: closes connection
 * - write: sends data to agent
 * - read: receives data from agent
 * 
 * @return Transport initialization structure for micro-ROS
 */
#ifdef RMW_UXRCE_TRANSPORT_CUSTOM
/**
 * @brief Get micro-ROS custom transport parameters
 * 
 * @return Transport parameters for micro-ROS
 */
rmw_uros_transport_params_t w5500_get_transport(void);

/**
 * @brief Open transport for micro-ROS
 * Required by micro-ROS custom transport API
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @return true if opened successfully, false otherwise
 */
bool cubemx_transport_open(struct uxrCustomTransport * transport);

/**
 * @brief Close transport for micro-ROS
 * Required by micro-ROS custom transport API
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @return true if closed successfully, false otherwise
 */
bool cubemx_transport_close(struct uxrCustomTransport * transport);

/**
 * @brief Write data over transport for micro-ROS
 * Required by micro-ROS custom transport API
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @param buf Buffer with data to write
 * @param len Length of data to write
 * @param err Error code (output parameter)
 * @return Number of bytes written
 */
size_t cubemx_transport_write(struct uxrCustomTransport* transport, uint8_t * buf, size_t len, uint8_t * err);

/**
 * @brief Read data from transport for micro-ROS
 * Required by micro-ROS custom transport API
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @param buf Buffer to store read data
 * @param len Maximum length to read
 * @param timeout Timeout in milliseconds
 * @param err Error code (output parameter)
 * @return Number of bytes read
 */
size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);
#endif // RMW_UXRCE_TRANSPORT_CUSTOM

#endif /* W5500_TRANSPORT_H_ */