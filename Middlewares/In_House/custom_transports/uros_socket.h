/**
 * @file uros_socket.h
 * @brief micro-ROS W5500 socket + discovery abstraction layer
 * @details Provides socket management, agent discovery, and robust connection handling
 */

#ifndef _UROS_SOCKET_H_
#define _UROS_SOCKET_H_

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h" /* CMSIS-RTOS v2 */

/* Socket configuration */
#define UROS_SOCKET_NUM             0
#define UROS_SOCKET_LOCAL_PORT      8888
#define UROS_AGENT_PORT             8888
#define UROS_DISCOVERY_PORT         8888
#define UROS_DISCOVERY_TIMEOUT_MS   1000
#define UROS_DISCOVERY_SIGNATURE    "uros-agent"
#define UROS_DISCOVERY_SIG_LEN      10
#define UROS_SOCKET_BUF_SIZE        1024

/* Connection retry configuration */
#define UROS_MAX_RECONNECT_ATTEMPTS 10     /* Maximum number of reconnection attempts */
#define UROS_INITIAL_RETRY_MS       100    /* Initial retry delay in ms */
#define UROS_MAX_RETRY_MS           5000   /* Maximum retry delay in ms */
#define UROS_RETRY_FACTOR           2      /* Exponential backoff multiplier */
#define UROS_HEARTBEAT_PERIOD_MS    5000   /* Heartbeat check interval when idle */

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
    UROS_ERR_CONNECT,
    UROS_ERR_SEND,
    UROS_ERR_RECEIVE,
    UROS_ERR_TIMEOUT,
    UROS_ERR_CLOSED
} uros_error_code_t;

typedef struct {
    uint8_t ip[4];
    uint16_t port;
} uros_agent_info_t;

/**
 * @brief Discover a micro-ROS agent on the local network
 * 
 * @param agent Pointer to struct that will receive agent IP and port
 * @return true if agent was found, false otherwise
 */
bool uros_agent_discovery(uros_agent_info_t *agent);

/**
 * @brief Establish TCP connection to agent
 * 
 * @param agent Pointer to agent IP and port
 * @return true if connected successfully
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
 * @brief Disconnect TCP connection
 * 
 * @return true if socket was closed successfully
 */
bool uros_close(void);

/**
 * @brief Send data through active socket
 * 
 * @param data Pointer to data buffer
 * @param len Number of bytes to send
 * @return Number of bytes sent, or 0 on error
 */
int32_t uros_send(const uint8_t *data, uint16_t len);

/**
 * @brief Receive data from active socket
 * 
 * @param data Buffer to receive data
 * @param len Max number of bytes to receive
 * @return Number of bytes received, or 0 on error
 */
int32_t uros_recv(uint8_t *data, uint16_t len);

/**
 * @brief Check if connection is still active
 * @details Performs a comprehensive connection status check
 * @return true if socket is connected
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

#endif /* _UROS_SOCKET_H_ */