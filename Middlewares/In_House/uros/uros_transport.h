/**
 * @file uros_transport.h
 * @brief W5500 Ethernet transport interface for micro-ROS
 * 
 * @details This transport implementation provides the necessary interface for
 *          integrating a W5500 Ethernet controller with the micro-ROS middleware.
 *          It implements the custom transport API required by micro-ROS and sits
 *          at the top of the networking stack hierarchy.
 * 
 * @note    The transport layer depends on uros_socket for communication with
 *          the micro-ROS agent and uses the custom transport API from micro-ROS.
 * 
 * @see     uros_socket.h for socket wrapper functions
 * @see     eth_config.h for micro-ROS and network configuration parameters
 */

#ifndef _UROS_TRANSPORT_H_
#define _UROS_TRANSPORT_H_

#include <stdbool.h>
#include <stdint.h>

/* Only compile if micro-ROS support is enabled */
#ifdef RMW_UXRCE_TRANSPORT_CUSTOM

#include <uxr/client/transport.h>
#include <rmw_microros/rmw_microros.h>

/* Include the socket wrapper API for network operations */
#include "w5500_socket.h"

/**
 * @brief Get micro-ROS custom transport parameters for integration with RMW
 * 
 * @details Creates and returns a transport parameters structure configured for
 *          W5500 Ethernet communication. This structure contains all necessary
 *          function pointers and configuration for micro-ROS to use the W5500
 *          transport as a custom transport layer.
 * 
 * @note    This is the primary function called by micro-ROS applications to
 *          set up the W5500 transport.
 * 
 * @return Transport parameters structure initialized for W5500
 */
rmw_uros_transport_params_t uros_get_transport(void);

/**
 * @brief Open transport for micro-ROS (required by micro-ROS custom transport API)
 * 
 * @details This function is called by the micro-ROS middleware to establish
 *          communication with the agent. It extracts connection parameters from
 *          the transport structure and initializes the W5500 hardware and network.
 * 
 * @note    This is part of the uxrCustomTransport API and should not be called
 *          directly by application code.
 * 
 * @param transport Pointer to uxrCustomTransport structure containing connection parameters
 * @return true if opened successfully, false otherwise
 */
bool uros_transport_open(struct uxrCustomTransport * transport);

/**
 * @brief Close transport for micro-ROS (required by micro-ROS custom transport API)
 * 
 * @details Closes the socket connection when micro-ROS is finished with the transport.
 * 
 * @note    This is part of the uxrCustomTransport API and should not be called
 *          directly by application code.
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @return true if closed successfully, false otherwise
 */
bool uros_transport_close(struct uxrCustomTransport * transport);

/**
 * @brief Write data over transport for micro-ROS (required by micro-ROS custom transport API)
 * 
 * @details Sends data to the micro-ROS agent. This function handles socket state
 *          checking and will attempt to reconnect if the connection was lost.
 * 
 * @note    This is part of the uxrCustomTransport API and should not be called
 *          directly by application code.
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @param buf Buffer containing data to write
 * @param len Length of data to write in bytes
 * @param err Output parameter to indicate error status (0 = success)
 * @return Number of bytes written, or 0 if error
 */
size_t uros_transport_write(struct uxrCustomTransport* transport, uint8_t * buf, size_t len, uint8_t * err);

/**
 * @brief Read data from transport for micro-ROS (required by micro-ROS custom transport API)
 * 
 * @details Receives data from the micro-ROS agent with timeout. This function
 *          will poll the socket until data is available or the timeout expires.
 *          It also handles connection loss and reconnection attempts.
 * 
 * @note    This is part of the uxrCustomTransport API and should not be called
 *          directly by application code.
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @param buf Buffer to store received data
 * @param len Maximum number of bytes to read
 * @param timeout Maximum time to wait for data in milliseconds
 * @param err Output parameter to indicate error status (0 = success)
 * @return Number of bytes read, or 0 if timeout/error
 */
size_t uros_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);

/**
 * @brief Initialize the micro-ROS transport with specific agent information
 * 
 * @details Sets up the W5500 transport to communicate with a specific micro-ROS agent
 * 
 * @param agent_ip IP address of the micro-ROS agent (4 bytes)
 * @param agent_port Port number of the micro-ROS agent
 * @return true if initialization was successful, false otherwise
 */
bool uros_transport_init(const uint8_t agent_ip[4], uint16_t agent_port);

#endif // RMW_UXRCE_TRANSPORT_CUSTOM

#endif /* _UROS_TRANSPORT_H_ */