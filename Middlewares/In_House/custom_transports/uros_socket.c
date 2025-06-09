/**
 * @file uros_socket.c
 * @brief micro-ROS socket + UDP discovery for W5500
 * @details Implements robust socket connection management with reconnection capabilities
 */

#include "uros_socket.h"
#include "w5500_socket.h"
#include <string.h>
/* Using CMSIS-RTOS v2 instead of direct HAL */

/* CMSIS-RTOS v2 objects */
static osMutexId_t socket_mutex;  /* Mutex to protect socket operations */
static const osMutexAttr_t socket_mutex_attr = {
    "socketMutex",             /* Mutex name */
    osMutexPrioInherit,       /* Priority inheritance */
    NULL,                     /* Memory for mutex control block */
    0U                        /* Size of mutex control block */
};

/* Connection state tracking */
static bool socket_connected = false;
static uros_connection_stats_t connection_stats = {0};

/**
 * @brief Initialize the socket module with RTOS objects
 * @return true if initialized successfully
 */
bool uros_socket_init(void)
{
    /* Create mutex for thread safety */
    socket_mutex = osMutexNew(&socket_mutex_attr);
    if (socket_mutex == NULL) {
        return false;
    }
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

bool uros_agent_discovery(uros_agent_info_t *agent)
{
    uint8_t tx_buf[] = "discover-micro-ros";
    uint8_t rx_buf[UROS_SOCKET_BUF_SIZE];
    uint8_t remote_ip[4];
    uint16_t remote_port;
    uint32_t start_tick = osKernelGetTickCount();
    uint8_t discovery_attempts = 0;
    bool result = false;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    
    /* Update connection state */
    connection_stats.state = UROS_CONN_DISCONNECTED;
    
    /* Create UDP socket for discovery */
    if (w5500_socket(UROS_SOCKET_NUM, Sn_MR_UDP, UROS_SOCKET_LOCAL_PORT, 0) != UROS_SOCKET_NUM) {
        connection_stats.last_error = UROS_ERR_SOCKET_CREATE;
        osMutexRelease(socket_mutex);
        return false;
    }
    
    /* Send discovery messages with multiple attempts */
    uint8_t broadcast_ip[4] = {255, 255, 255, 255};
    
    /* Send initial discovery request */
    w5500_sendto(UROS_SOCKET_NUM, tx_buf, sizeof(tx_buf), broadcast_ip, UROS_DISCOVERY_PORT);
    discovery_attempts++;
    
    while ((osKernelGetTickCount() - start_tick) < UROS_DISCOVERY_TIMEOUT_MS)
    {
        /* Resend discovery requests periodically */
        if (discovery_attempts < 5 && 
            (osKernelGetTickCount() - start_tick) > (discovery_attempts * 200)) {
            w5500_sendto(UROS_SOCKET_NUM, tx_buf, sizeof(tx_buf), broadcast_ip, UROS_DISCOVERY_PORT);
            discovery_attempts++;
        }
        
        /* Check for responses */
        int32_t len = w5500_recvfrom(UROS_SOCKET_NUM, rx_buf, sizeof(rx_buf), remote_ip, &remote_port);
        if (len >= UROS_DISCOVERY_SIG_LEN &&
            memcmp(rx_buf, UROS_DISCOVERY_SIGNATURE, UROS_DISCOVERY_SIG_LEN) == 0)
        {
            memcpy(agent->ip, remote_ip, 4);
            agent->port = UROS_AGENT_PORT;
            w5500_close(UROS_SOCKET_NUM);
            result = true;
            break;
        }
        
        /* Small delay to prevent CPU hammering */
        osMutexRelease(socket_mutex);
        osDelay(10);
        osMutexAcquire(socket_mutex, osWaitForever);
    }

    if (!result) {
        w5500_close(UROS_SOCKET_NUM);
        connection_stats.last_error = UROS_ERR_TIMEOUT;
    }
    
    osMutexRelease(socket_mutex);
    return result;
}

bool uros_connect(const uros_agent_info_t *agent)
{
    bool result = false;
    
    /* Acquire mutex for thread safety */
    osMutexAcquire(socket_mutex, osWaitForever);
    
    /* Update connection state */
    connection_stats.state = UROS_CONN_CONNECTING;
    
    /* Create TCP socket */
    if (w5500_socket(UROS_SOCKET_NUM, Sn_MR_TCP, UROS_SOCKET_LOCAL_PORT, 0) != UROS_SOCKET_NUM) {
        connection_stats.last_error = UROS_ERR_SOCKET_CREATE;
        connection_stats.state = UROS_CONN_ERROR;
        osMutexRelease(socket_mutex);
        return false;
    }

    /* Connect to the agent */
    if (w5500_connect(UROS_SOCKET_NUM, agent->ip, agent->port) != SOCK_OK) {
        w5500_close(UROS_SOCKET_NUM);
        connection_stats.last_error = UROS_ERR_CONNECT;
        connection_stats.state = UROS_CONN_ERROR;
        osMutexRelease(socket_mutex);
        return false;
    }

    /* Update connection state and activity timestamp */
    socket_connected = true;
    connection_stats.state = UROS_CONN_CONNECTED;
    connection_stats.last_activity_time = osKernelGetTickCount();
    connection_stats.last_error = UROS_ERR_NONE;
    result = true;
    
    /* Release mutex */
    osMutexRelease(socket_mutex);
    return result;
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
            /* Discovery successful */
        } else {
            /* Set default agent IP and port if discovery fails */
            agent->ip[0] = 192;
            agent->ip[1] = 168;
            agent->ip[2] = 1;
            agent->ip[3] = 10;
            agent->port = UROS_AGENT_PORT;
        }
    }
    
    /* If already connected, close the connection first */
    osMutexAcquire(socket_mutex, osWaitForever);
    bool was_connected = socket_connected;
    osMutexRelease(socket_mutex);
    
    if (was_connected) {
        uros_close();
    }
    
    /* Try reconnecting with exponential backoff */
    uint16_t retry_delay = UROS_INITIAL_RETRY_MS;
    
    for (uint8_t attempt = 0; attempt < UROS_MAX_RECONNECT_ATTEMPTS; attempt++) {
        /* Try to connect using parameters */
        if (uros_connect(agent)) {
            /* Connection successful */
            osMutexAcquire(socket_mutex, osWaitForever);
            connection_stats.successful_reconnects++;
            osMutexRelease(socket_mutex);
            result = true;
            break;
        }
        
        /* Apply exponential backoff delay */
        osDelay(retry_delay);
        retry_delay *= UROS_RETRY_FACTOR;
        
        /* Cap the maximum retry delay */
        if (retry_delay > UROS_MAX_RETRY_MS) {
            retry_delay = UROS_MAX_RETRY_MS;
        }
    }
    
    return result;
}

bool uros_close(void)
{
    osMutexAcquire(socket_mutex, osWaitForever);
    
    w5500_close(UROS_SOCKET_NUM);
    socket_connected = false;
    connection_stats.state = UROS_CONN_DISCONNECTED;
    
    osMutexRelease(socket_mutex);
    return true;
}

bool uros_send(const uint8_t *buf, size_t len)
{
    bool result = false;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    
    if (!socket_connected) {
        connection_stats.last_error = UROS_ERR_NOT_CONNECTED;
        osMutexRelease(socket_mutex);
        return false;
    }

    /* Send the data */
    int32_t send_result = w5500_send(UROS_SOCKET_NUM, buf, len);
    if (send_result <= 0) {
        socket_connected = false;
        connection_stats.last_error = UROS_ERR_SEND;
        connection_stats.state = UROS_CONN_ERROR;
        osMutexRelease(socket_mutex);
        return false;
    }
    
    /* Update connection stats */
    connection_stats.bytes_sent += send_result;
    connection_stats.last_activity_time = osKernelGetTickCount();
    connection_stats.state = UROS_CONN_CONNECTED;
    result = true;
    
    osMutexRelease(socket_mutex);
    return result;
}

int32_t uros_recv(uint8_t *buf, size_t len)
{
    int32_t result;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    
    if (!socket_connected) {
        connection_stats.last_error = UROS_ERR_NOT_CONNECTED;
        osMutexRelease(socket_mutex);
        return -1;
    }

    /* Receive the data */
    result = w5500_recv(UROS_SOCKET_NUM, buf, len);
    
    /* Handle socket error or closed condition */
    if (result < 0) {
        socket_connected = false;
        connection_stats.last_error = UROS_ERR_RECV;
        connection_stats.state = UROS_CONN_ERROR;
    }
    else if (result > 0) {
        /* Update connection stats on successful receive */
        connection_stats.bytes_received += result;
        connection_stats.last_activity_time = osKernelGetTickCount();
        connection_stats.state = UROS_CONN_CONNECTED;
    }
    
    osMutexRelease(socket_mutex);
    return result;
}

bool uros_status(void)
{
    bool connected;
    
    osMutexAcquire(socket_mutex, osWaitForever);
    connected = socket_connected;
    osMutexRelease(socket_mutex);
    
    return connected;
}

bool uros_heartbeat(uros_agent_info_t *agent)
{
    static uint32_t last_heartbeat_time = 0;
    uint32_t current_time = osKernelGetTickCount();
    uint32_t last_activity;
    bool connected;
    bool result = true;
    
    /* Only perform heartbeat check if enough time has passed since last check */
    if ((current_time - last_heartbeat_time) < UROS_HEARTBEAT_PERIOD_MS) {
        return uros_status();
    }
    
    /* Update heartbeat timestamp */
    last_heartbeat_time = current_time;
    
    /* Get connection status and last activity time with proper synchronization */
    osMutexAcquire(socket_mutex, osWaitForever);
    last_activity = connection_stats.last_activity_time;
    connected = socket_connected;
    osMutexRelease(socket_mutex);
    
    /* Check connection status */
    if (!connected) {
        /* If disconnected, attempt to reconnect */
        return uros_reconnect(agent);
    }
    
    /* If connection has been idle for too long, perform a status check */
    if ((current_time - last_activity) > (UROS_HEARTBEAT_PERIOD_MS * 3)) {
        /* Connection has been idle, perform an active check */
        if (!uros_status()) {
            /* Failed status check, attempt to reconnect */
            result = uros_reconnect(agent);
        }
    }
    
    return result;
}