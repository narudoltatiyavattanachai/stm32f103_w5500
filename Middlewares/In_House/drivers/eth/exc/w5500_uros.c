/**
 * @file w5500_uros.c
 * @brief micro-ROS socket + UDP discovery for W5500
 * @details Implements robust socket communication with agent discovery and
 *          reconnection capabilities for micro-ROS over UDP
 */

#include <string.h>
#include <stdio.h>
#include "w5500_uros.h"
#include "w5500_socket.h"
#include <uxr/client/transport.h>

/* Debug output control */
#ifdef W5500_DEBUG
#define DEBUG_PRINT(...)    printf("UROS: " __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/* CMSIS-RTOS v2 objects */
static osMutexId_t socket_mutex;  /* Mutex to protect socket operations */
static const osMutexAttr_t socket_mutex_attr = {
    "urosSocketMutex",        /* Mutex name */
    osMutexPrioInherit,       /* Priority inheritance */
    NULL,                     /* Memory for mutex control block */
    0U                        /* Size of mutex control block */
};

/* Connection state tracking */
static bool socket_created = false;
static uros_connection_stats_t connection_stats = {0};
static uint8_t current_agent_ip[4] = {0};
static uint16_t current_agent_port = 0;

/**
 * @brief Initialize the socket module with RTOS objects
 * @return true if initialized successfully
 */
bool uros_socket_init(void)
{
    /* Create mutex for thread safety */
    socket_mutex = osMutexNew(&socket_mutex_attr);
    if (socket_mutex == NULL) {
        DEBUG_PRINT("Failed to create socket mutex\r\n");
        return false;
    }
    
    /* Initialize connection statistics */
    memset(&connection_stats, 0, sizeof(connection_stats));
    connection_stats.state = UROS_CONN_DISCONNECTED;
    connection_stats.last_error = UROS_ERR_NONE;
    
    DEBUG_PRINT("Socket module initialized\r\n");
    return true;
}

/* Get connection statistics */
const uros_connection_stats_t* uros_get_stats(void)
{
    static uros_connection_stats_t stats_copy;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    /* Create a copy to avoid race conditions */
    memcpy(&stats_copy, &connection_stats, sizeof(uros_connection_stats_t));
    osMutexRelease(socket_mutex);
    
    return &stats_copy;
}

/**
 * @brief Discover a micro-ROS agent on the local network
 * @param agent Pointer to struct that will receive agent IP and port
 * @return true if agent was found, false otherwise
 */
bool uros_agent_discovery(uros_agent_info_t *agent)
{
    uint8_t tx_buf[] = "discover-micro-ros";
    uint8_t rx_buf[ETH_CONFIG_UROS_BUF_SIZE];
    uint8_t remote_ip[4];
    uint16_t remote_port;
    uint32_t start_tick = osKernelGetTickCount();
    uint8_t discovery_attempts = 0;
    bool result = false;
    
    if (agent == NULL) {
        DEBUG_PRINT("Agent discovery: NULL agent pointer\r\n");
        return false;
    }
    
    osMutexAcquire(socket_mutex, osWaitForever);
    
    /* Update connection state */
    connection_stats.state = ETH_CONFIG_UROS_CONN_DISCONNECTED;
    
    /* Close any existing socket first */
    if (socket_created) {
        w5500_close(ETH_CONFIG_UROS_SOCKET);
        socket_created = false;
    }
    
    /* Create UDP socket for discovery */
    if (w5500_socket(ETH_CONFIG_UROS_SOCKET, Sn_MR_UDP, ETH_CONFIG_UROS_LOCAL_PORT, 0) != ETH_CONFIG_UROS_SOCKET) {
        connection_stats.last_error = UROS_ERR_SOCKET_CREATE;
        DEBUG_PRINT("Agent discovery: Socket creation failed\r\n");
        osMutexRelease(socket_mutex);
        return false;
    }
    
    socket_created = true;
    
    /* Send discovery messages with multiple attempts */
    uint8_t broadcast_ip[4] = {255, 255, 255, 255};
    
    /* Send initial discovery request */
    DEBUG_PRINT("Agent discovery: Broadcasting discovery request\r\n");
    w5500_sendto(ETH_CONFIG_UROS_SOCKET, tx_buf, sizeof(tx_buf), broadcast_ip, ETH_CONFIG_UROS_DISC_PORT);
    discovery_attempts++;
    
    while ((osKernelGetTickCount() - start_tick) < ETH_CONFIG_UROS_DISC_TIMEOUT)
    {
        /* Resend discovery requests periodically */
        if (discovery_attempts < 5 && 
            (osKernelGetTickCount() - start_tick) > (discovery_attempts * 200)) {
            w5500_sendto(ETH_CONFIG_UROS_SOCKET, tx_buf, sizeof(tx_buf), broadcast_ip, ETH_CONFIG_UROS_DISC_PORT);
            discovery_attempts++;
            DEBUG_PRINT("Agent discovery: Resending discovery request (attempt %d)\r\n", discovery_attempts);
        }
        
        /* Check for responses */
        int32_t len = w5500_recvfrom(ETH_CONFIG_UROS_SOCKET, rx_buf, sizeof(rx_buf), remote_ip, &remote_port);
        if (len >= ETH_CONFIG_UROS_DISC_SIG_LEN &&
            memcmp(rx_buf, ETH_CONFIG_UROS_DISC_SIG, ETH_CONFIG_UROS_DISC_SIG_LEN) == 0)
        {
            memcpy(agent->ip, remote_ip, 4);
            agent->port = ETH_CONFIG_UROS_AGENT_PORT;
            DEBUG_PRINT("Agent discovery: Found agent at %d.%d.%d.%d:%d\r\n", 
                      remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], agent->port);
            result = true;
            break;
        }
        
        /* Small delay to prevent CPU hammering */
        osMutexRelease(socket_mutex);
        HAL_Delay(10);
        osMutexAcquire(socket_mutex, osWaitForever);
    }
    
    /* Always close the discovery socket when done */
    w5500_close(ETH_CONFIG_UROS_SOCKET);
    socket_created = false;
    
    if (!result) {
        connection_stats.last_error = UROS_ERR_TIMEOUT;
        DEBUG_PRINT("Agent discovery: No agent found (timeout)\r\n");
    }
    
    osMutexRelease(socket_mutex);
    return result;
}


    discovery_attempts++;
    
    while ((osKernelGetTickCount() - start_tick) < ETH_CONFIG_UROS_DISC_TIMEOUT)
    {
        /* Resend discovery requests periodically */
        if (discovery_attempts < 5 && 
            (osKernelGetTickCount() - start_tick) > (discovery_attempts * 200)) {
            w5500_sendto(ETH_CONFIG_UROS_SOCKET, tx_buf, sizeof(tx_buf), broadcast_ip, ETH_CONFIG_UROS_DISC_PORT);
            discovery_attempts++;
        }
        
        /* Check for responses */
        int32_t len = w5500_recvfrom(ETH_CONFIG_UROS_SOCKET, rx_buf, sizeof(rx_buf), remote_ip, &remote_port);
        if (len >= ETH_CONFIG_UROS_DISC_SIG_LEN &&
            memcmp(rx_buf, ETH_CONFIG_UROS_DISC_SIG, ETH_CONFIG_UROS_DISC_SIG_LEN) == 0)
        {
            memcpy(agent->ip, remote_ip, 4);
            agent->port = ETH_CONFIG_UROS_AGENT_PORT;
            w5500_close(ETH_CONFIG_UROS_SOCKET);
            result = true;
            break;
        }
        
        /* Small delay to prevent CPU hammering */
        osMutexRelease(socket_mutex);
        HAL_Delay(10);
        osMutexAcquire(socket_mutex, osWaitForever);
    }

    if (!result) {
        w5500_close(ETH_CONFIG_UROS_SOCKET);
        connection_stats.last_error = UROS_ERR_TIMEOUT;
    }
    
    osMutexRelease(socket_mutex);
    return result;
}

bool uros_connect(const uros_agent_info_t *agent)
{
    /* Acquire mutex for thread safety */
    osMutexAcquire(socket_mutex, osWaitForever);
    
    /* Update connection state */
    connection_stats.state = UROS_CONN_CONNECTING;
    
    /* Close any existing socket first */
    if (socket_created) {
        w5500_close(ETH_CONFIG_UROS_SOCKET);
        socket_created = false;
    }
    
    /* Create UDP socket */
    if (w5500_socket(ETH_CONFIG_UROS_SOCKET, Sn_MR_UDP, ETH_CONFIG_UROS_LOCAL_PORT, 0) != ETH_CONFIG_UROS_SOCKET) {
        connection_stats.last_error = UROS_ERR_SOCKET_CREATE;
        connection_stats.state = UROS_CONN_ERROR;
        DEBUG_PRINT("Failed to create UDP socket\r\n");
        osMutexRelease(socket_mutex);
        return false;
    }

    /* Store the agent IP and port for later use with sendto */
    memcpy(current_agent_ip, agent->ip, 4);
    current_agent_port = agent->port;
    
    DEBUG_PRINT("UDP socket created for agent %d.%d.%d.%d:%d\r\n",
               agent->ip[0], agent->ip[1], agent->ip[2], agent->ip[3], agent->port);

    /* Update connection state and activity timestamp */
    socket_created = true;
    connection_stats.state = UROS_CONN_CONNECTED;
    connection_stats.last_activity_time = osKernelGetTickCount();
    connection_stats.last_error = UROS_ERR_NONE;
    
    /* Release mutex */
    osMutexRelease(socket_mutex);
    return true;
}

bool uros_reconnect(uros_agent_info_t *agent)
{
    bool result = false;
    
    /* Acquire mutex for thread safety */
    osMutexAcquire(socket_mutex, osWaitForever);
    
    /* Track reconnection attempts for statistics */
    connection_stats.reconnect_attempts++;
    
    /* Release mutex during potentially long operations */
    osMutexRelease(socket_mutex);
    
    /* Check if we already have agent info set and apply defaults if needed */
    if (agent->ip[0] == 0 && agent->ip[1] == 0 && 
        agent->ip[2] == 0 && agent->ip[3] == 0)
    {
        /* First try to discover an agent */
        if (uros_agent_discovery(agent)) {
            DEBUG_PRINT("Reconnect: Agent discovered successfully\r\n");
        } else {
            /* Set default agent IP and port if discovery fails */
            agent->ip[0] = 192;
            agent->ip[1] = 168;
            agent->ip[2] = 1;
            agent->ip[3] = 100;
            agent->port = ETH_CONFIG_UROS_AGENT_PORT;
            DEBUG_PRINT("Reconnect: Using default agent %d.%d.%d.%d:%d\r\n",
                      agent->ip[0], agent->ip[1], agent->ip[2], agent->ip[3], agent->port);
        }
    }
    
    /* If already have a socket, close it first */
    osMutexAcquire(socket_mutex, osWaitForever);
    bool had_socket = socket_created;
    osMutexRelease(socket_mutex);
    
    if (had_socket) {
        DEBUG_PRINT("Reconnect: Closing existing socket\r\n");
        uros_close();
    }
    
    /* Try reconnecting with exponential backoff */
    uint16_t retry_delay = ETH_CONFIG_UROS_INIT_RETRY_MS;
    
    for (uint8_t attempt = 0; attempt < ETH_CONFIG_UROS_MAX_RETRY; attempt++) {
        DEBUG_PRINT("Reconnect: Attempt %d\r\n", attempt + 1);
        
        /* Try to connect using parameters */
        if (uros_connect(agent)) {
            /* Connection successful */
            osMutexAcquire(socket_mutex, osWaitForever);
            connection_stats.successful_reconnects++;
            osMutexRelease(socket_mutex);
            DEBUG_PRINT("Reconnect: Successful\r\n");
            result = true;
            break;
        }
        
        /* Apply exponential backoff delay */
        DEBUG_PRINT("Reconnect: Failed, retrying in %d ms\r\n", retry_delay);
        HAL_Delay(retry_delay);
        retry_delay *= ETH_CONFIG_UROS_RETRY_FACTOR;
        
        /* Cap the maximum retry delay */
        if (retry_delay > ETH_CONFIG_UROS_MAX_RETRY_MS) {
            retry_delay = ETH_CONFIG_UROS_MAX_RETRY_MS;
        }
    }
    
    if (!result) {
        DEBUG_PRINT("Reconnect: All attempts failed\r\n");
    }
    
    return result;
}

bool uros_close(void)
{
    osMutexAcquire(socket_mutex, osWaitForever);
    
    if (socket_created) {
        DEBUG_PRINT("Closing socket %d\r\n", ETH_CONFIG_UROS_SOCKET);
        w5500_close(ETH_CONFIG_UROS_SOCKET);
        socket_created = false;
    }
    connection_stats.state = UROS_CONN_DISCONNECTED;
    
    osMutexRelease(socket_mutex);
    return true;
}

int32_t uros_send(const uint8_t *buf, uint16_t len)
{
    int32_t send_result;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    
    if (!socket_created) {
        DEBUG_PRINT("Send error: Socket not created\r\n");
        connection_stats.last_error = UROS_ERR_NOT_CONNECTED;
        osMutexRelease(socket_mutex);
        return -1;
    }

    /* Send the data using sendto with the stored agent info */
    send_result = w5500_sendto(ETH_CONFIG_UROS_SOCKET, buf, len, 
                             current_agent_ip, current_agent_port);
                             
    if (send_result < 0) {
        DEBUG_PRINT("Send error: Failed to send data (%ld)\r\n", send_result);
        connection_stats.last_error = UROS_ERR_SEND;
        connection_stats.state = UROS_CONN_ERROR;
    }
    else if (send_result > 0) {
        /* Update connection stats */
        connection_stats.bytes_sent += send_result;
        connection_stats.last_activity_time = osKernelGetTickCount();
        connection_stats.state = UROS_CONN_CONNECTED;
    }
    
    osMutexRelease(socket_mutex);
    return send_result;
}

int32_t uros_recv(uint8_t *buf, uint16_t len)
{
    int32_t result;
    uint8_t remote_ip[4];
    uint16_t remote_port;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    
    if (!socket_created) {
        DEBUG_PRINT("Receive error: Socket not created\r\n");
        connection_stats.last_error = UROS_ERR_NOT_CONNECTED;
        osMutexRelease(socket_mutex);
        return -1;
    }

    /* Receive the data using recvfrom since we're using UDP */
    result = w5500_recvfrom(ETH_CONFIG_UROS_SOCKET, buf, len, remote_ip, &remote_port);
    
    /* Handle socket error or closed condition */
    if (result < 0) {
        DEBUG_PRINT("Receive error: Failed to receive data (%ld)\r\n", result);
        connection_stats.last_error = UROS_ERR_RECV;
        connection_stats.state = UROS_CONN_ERROR;
    }
    else if (result > 0) {
        /* Update connection stats on successful receive */
        connection_stats.bytes_received += result;
        connection_stats.last_activity_time = osKernelGetTickCount();
        connection_stats.state = UROS_CONN_CONNECTED;
        
        /* Optionally verify sender matches our agent */
        if (memcmp(remote_ip, current_agent_ip, 4) != 0 || remote_port != current_agent_port) {
            DEBUG_PRINT("Received data from non-agent source: %d.%d.%d.%d:%d\r\n",
                      remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
        }
    }
    
    osMutexRelease(socket_mutex);
    return result;
}

bool uros_status(void)
{
    bool status;
    uint8_t sock_status;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    
    if (!socket_created) {
        status = false;
    } else {
        /* Check if the socket is still open and in UDP mode */
        sock_status = w5500_status(ETH_CONFIG_UROS_SOCKET);
        status = (sock_status == SOCK_UDP);
        
        /* Update connection state if socket is no longer valid */
        if (!status) {
            DEBUG_PRINT("Socket status check: Socket not in UDP mode (%d)\r\n", sock_status);
            connection_stats.state = UROS_CONN_ERROR;
            socket_created = false;
        }
    }
    
    osMutexRelease(socket_mutex);
    return status;
}

bool uros_heartbeat(uros_agent_info_t *agent)
{
    static uint32_t last_heartbeat_time = 0;
    uint32_t current_time = osKernelGetTickCount();
    uint32_t last_activity;
    bool socket_ok;
    bool result = true;
    
    /* Only perform heartbeat check if enough time has passed since last check */
    if ((current_time - last_heartbeat_time) < ETH_CONFIG_UROS_HEARTBEAT_MS) {
        return uros_status();
    }
    
    /* Update heartbeat timestamp */
    last_heartbeat_time = current_time;
    
    DEBUG_PRINT("Heartbeat check\r\n");
    
    /* Get socket status and last activity time with proper synchronization */
    osMutexAcquire(socket_mutex, osWaitForever);
    last_activity = connection_stats.last_activity_time;
    socket_ok = socket_created;
    osMutexRelease(socket_mutex);
    
    /* Check socket status */
    if (!socket_ok) {
        /* If socket not created, attempt to reconnect */
        DEBUG_PRINT("Heartbeat: Socket not created, attempting reconnect\r\n");
        return uros_reconnect(agent);
    }
    
    /* If connection has been idle for too long, perform a status check */
    if ((current_time - last_activity) > (ETH_CONFIG_UROS_HEARTBEAT_MS * 3)) {
        DEBUG_PRINT("Heartbeat: Connection idle for %lu ms, checking status\r\n", 
                   (current_time - last_activity));
                   
        /* Connection has been idle, perform an active status check */
        if (!uros_status()) {
            /* Failed status check, attempt to reconnect */
            DEBUG_PRINT("Heartbeat: Status check failed, attempting reconnect\r\n");
            result = uros_reconnect(agent);
        } else {
            DEBUG_PRINT("Heartbeat: Status check passed\r\n");
        }
    } else {
        DEBUG_PRINT("Heartbeat: Connection active, last activity %lu ms ago\r\n",
                  (current_time - last_activity));
    }
    
    return result;
}

/* --- micro-ROS Transport Interface Functions --- */

/**
 * @brief Opens a transport connection for micro-ROS
 * @param transport Pointer to transport structure
 * @return true if opened successfully
 */
bool cubemx_transport_open(struct uxrCustomTransport *transport)
{
    const char *agent_ip = (const char*)transport->args;
    uros_agent_info_t agent_info = {0};
    
    DEBUG_PRINT("Transport: Opening connection\r\n");
    
    /* Initialize the socket module if not already initialized */
    if (!uros_socket_init()) {
        DEBUG_PRINT("Transport: Failed to initialize socket module\r\n");
        return false;
    }
    
    /* Parse IP address from string to bytes if provided */
    if (agent_ip != NULL && strlen(agent_ip) > 0) {
        sscanf(agent_ip, "%hhu.%hhu.%hhu.%hhu", 
               &agent_info.ip[0], &agent_info.ip[1], 
               &agent_info.ip[2], &agent_info.ip[3]);
        agent_info.port = ETH_CONFIG_UROS_AGENT_PORT;
        DEBUG_PRINT("Transport: Using provided agent IP %d.%d.%d.%d:%d\r\n",
                  agent_info.ip[0], agent_info.ip[1], 
                  agent_info.ip[2], agent_info.ip[3],
                  agent_info.port);
    } else {
        /* Try to discover an agent */
        DEBUG_PRINT("Transport: No agent IP provided, discovering agent\r\n");
        if (!uros_agent_discovery(&agent_info)) {
            DEBUG_PRINT("Transport: Agent discovery failed\r\n");
            return false;
        }
    }
    
    /* Connect to the agent */
    if (!uros_connect(&agent_info)) {
        DEBUG_PRINT("Transport: Failed to connect to agent\r\n");
        return false;
    }
    
    DEBUG_PRINT("Transport: Successfully opened connection to agent\r\n");
    return true;
}

/**
 * @brief Closes a transport connection for micro-ROS
 * @param transport Pointer to transport structure
 * @return true if closed successfully
 */
bool cubemx_transport_close(struct uxrCustomTransport *transport)
{
    (void) transport;
    
    DEBUG_PRINT("Transport: Closing connection\r\n");
    return uros_close();
}

/**
 * @brief Writes data through the transport for micro-ROS
 * @param transport Pointer to transport structure
 * @param buf Data buffer to write
 * @param len Length of data to write
 * @param err Error code pointer
 * @return Number of bytes written
 */
size_t cubemx_transport_write(struct uxrCustomTransport *transport, 
                             uint8_t *buf, size_t len, uint8_t *err)
{
    (void) transport;
    (void) err;
    
    int32_t bytes_sent = uros_send(buf, len);
    return (bytes_sent > 0) ? bytes_sent : 0;
}

/**
 * @brief Reads data from the transport for micro-ROS
 * @param transport Pointer to transport structure
 * @param buf Buffer to store read data
 * @param len Maximum length to read
 * @param timeout Timeout in milliseconds
 * @param err Error code pointer
 * @return Number of bytes read
 */
size_t cubemx_transport_read(struct uxrCustomTransport *transport,
                            uint8_t *buf, size_t len, int timeout, uint8_t *err)
{
    (void) transport;
    (void) timeout;  /* We're not implementing timeout handling in this version */
    (void) err;
    
    int32_t bytes_recv = uros_recv(buf, len);
    return (bytes_recv > 0) ? bytes_recv : 0;
}