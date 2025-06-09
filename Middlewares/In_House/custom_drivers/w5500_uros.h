/**
 * @file w5500_uros.h
 * @brief micro-ROS W5500 socket + discovery abstraction layer
 * 
 * @details Provides socket management, agent discovery, and robust connection handling
 *          as a wrapper around the base w5500_socket layer. This module implements
 *          the additional functionality needed for micro-ROS agent discovery and
 *          connection management beyond what the base socket layer provides.
 * 
 * @note    This module depends on w5500_socket.h for low-level socket operations
 *          and ip_config.h for micro-ROS specific configuration parameters.
 * 
 * @see     w5500_socket.h for base socket operations
 * @see     ip_config.h for network, socket assignments, and micro-ROS configuration
 */

#ifndef _W5500_UROS_H_
#define _W5500_UROS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cmsis_os2.h"      /* CMSIS-RTOS v2 */
#include "w5500_socket.h"  /* Base socket layer */
#include "ip_config.h"     /* Socket assignments and network config */

/* Connection health status */
typedef enum {
    UROS_CONN_DISCONNECTED = 0,
    UROS_CONN_CONNECTING, 
    UROS_CONN_CONNECTED,
    UROS_CONN_ERROR
} uros_connection_state_t;

/* Error codes for diagnostics */
typedef enum {
    UROS_ERR_NONE = 0,
    UROS_ERR_SOCKET_CREATE,
    UROS_ERR_NOT_CONNECTED,
    UROS_ERR_SEND,
    UROS_ERR_RECV,
    UROS_ERR_TIMEOUT,
    UROS_ERR_CLOSED
} uros_error_code_t;

/* Agent information structure */
typedef struct {
    uint8_t ip[4];          /* Agent IP address */
    uint16_t port;          /* Agent port number */
} uros_agent_info_t;

/* Connection statistics structure */
typedef struct {
    uros_connection_state_t state;            /* Current connection state */
    uros_error_code_t last_error;             /* Last error code */
    uint32_t bytes_sent;                      /* Total bytes sent */
    uint32_t bytes_received;                  /* Total bytes received */
    uint32_t last_activity_time;              /* Timestamp of last activity */
    uint16_t reconnect_attempts;              /* Count of reconnection attempts */
    uint16_t successful_reconnects;           /* Count of successful reconnections */
} uros_connection_stats_t;

/**
 * @brief Discover a micro-ROS agent on the local network using UDP broadcast
 * 
 * @param agent Pointer to struct that will receive agent IP and port
 * @return true if agent was found, false otherwise
 */
bool uros_agent_discovery(uros_agent_info_t *agent);

/**
 * @brief Establish UDP communication with agent
 * 
 * @param agent Pointer to agent IP and port
 * @return true if socket creation successful
 */
bool uros_connect(const uros_agent_info_t *agent);

/**
 * @brief Connect or reconnect to agent using default values if needed
 * 
 * @param agent Pointer to agent info - will be populated with defaults if empty
 * @return true if connected successfully, false otherwise
 */
bool uros_reconnect(uros_agent_info_t *agent);

/**
 * @brief Close the UDP socket
 * 
 * @return true if socket was closed successfully
 */
bool uros_close(void);

/**
 * @brief Send data through UDP socket to the agent
 * 
 * @param data Pointer to data buffer
 * @param len Number of bytes to send
 * @return Number of bytes sent, or negative value on error
 */
int32_t uros_send(const uint8_t *data, uint16_t len);

/**
 * @brief Receive data from the UDP socket
 * 
 * @param data Buffer to receive data
 * @param len Max number of bytes to receive
 * @return Number of bytes received, or negative value on error
 */
int32_t uros_recv(uint8_t *data, uint16_t len);

/**
 * @brief Check if connection is still active
 * @details Performs a socket status check
 * @return true if socket is created and operational
 */
bool uros_status(void);

/**
 * @brief Initialize socket module with CMSIS-RTOS v2 resources
 * @return true if initialization successful, false otherwise
 */
bool uros_socket_init(void);

/**
 * @brief Get connection statistics
 * @return Pointer to connection statistics structure
 */
const uros_connection_stats_t* uros_get_stats(void);

/**
 * @brief Perform a heartbeat check on the connection
 * @details Checks connection health and attempts proactive reconnection if needed
 * @param agent Agent information for reconnection if needed
 * @return true if connection is healthy or was successfully restored
 */
bool uros_heartbeat(uros_agent_info_t *agent);

/* micro-ROS transport interface functions */

/**
 * @brief Open transport for micro-ROS
 * @param transport Pointer to transport structure
 * @return true if opened successfully
 * 
 * @note To be registered with rmw_uros_set_custom_transport
 */
bool cubemx_transport_open(struct uxrCustomTransport * transport);

/**
 * @brief Close transport for micro-ROS
 * @param transport Pointer to transport structure
 * @return true if closed successfully
 * 
 * @note To be registered with rmw_uros_set_custom_transport
 */
bool cubemx_transport_close(struct uxrCustomTransport * transport);

/**
 * @brief Write data through micro-ROS transport
 * @param transport Pointer to transport structure
 * @param buf Data buffer to write
 * @param len Length of data to write
 * @param err Error code pointer
 * @return Number of bytes written
 * 
 * @note To be registered with rmw_uros_set_custom_transport
 */
size_t cubemx_transport_write(struct uxrCustomTransport* transport, 
                             uint8_t* buf, size_t len, uint8_t* err);

/**
 * @brief Read data from micro-ROS transport
 * @param transport Pointer to transport structure
 * @param buf Buffer to store read data
 * @param len Maximum length to read
 * @param timeout Timeout in milliseconds
 * @param err Error code pointer
 * @return Number of bytes read
 * 
 * @note To be registered with rmw_uros_set_custom_transport
 */
size_t cubemx_transport_read(struct uxrCustomTransport* transport,
                            uint8_t* buf, size_t len, int timeout, uint8_t* err);

#endif /* _W5500_UROS_H_ */