/**
 * @file uros_transport.c
 * @brief W5500 Ethernet transport implementation for micro-ROS
 *        Specialized module for micro-ROS integration
 * 
 * @details This file implements the custom transport API required by micro-ROS
 *          for the W5500 Ethernet controller. It provides the necessary functions
 *          to integrate with the micro-ROS middleware, handling communication
 *          with the micro-ROS agent over TCP/IP.
 * 
 * @author Your Name
 * @date 2025-06-09
 */

/* Includes ------------------------------------------------------------------*/
#include "uros_transport.h"
#include "uros_socket.h"
#include <string.h>
#include "cmsis_os2.h" /* CMSIS-RTOS v2 */

/* Only compile if micro-ROS support is enabled */
#ifdef RMW_UXRCE_TRANSPORT_CUSTOM

/* Debug print macro - comment out to disable debug prints */
#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
#include <stdio.h>  /* For printf */
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* No print */
#endif

/* Private variables ---------------------------------------------------------*/
static uros_agent_info_t agent_info = {0}; /* micro-ROS agent information */

/* ==========================================================================
 * MICRO-ROS TRANSPORT INTERFACE IMPLEMENTATION
 * ==========================================================================*/

/**
 * @brief Opens transport for micro-ROS
 * 
 * @details Reports status of the micro-ROS socket connection
 * 
 * @param transport Pointer to rmw_uros_transport structure
 * @return RMW_UROS_ERROR_OK if socket is open, RMW_UROS_ERROR_GENERAL otherwise
 */
static int8_t uros_transport_internal_open(struct rmw_uros_transport * transport)
{
    (void) transport; /* Unused parameter */
    
    return uros_status() ? RMW_UROS_ERROR_OK : RMW_UROS_ERROR_GENERAL;
}

/**
 * @brief Closes transport for micro-ROS
 * 
 * @details Closes the micro-ROS socket connection
 * 
 * @param transport Pointer to rmw_uros_transport structure
 * @return true if socket was closed successfully, false otherwise
 */
static bool uros_transport_internal_close(struct rmw_uros_transport * transport)
{
    (void) transport; /* Unused parameter */
    return uros_close();
}

/**
 * @brief Write data to micro-ROS transport
 * 
 * @details Sends data over socket connection if established
 * 
 * @param transport Pointer to rmw_uros_transport structure
 * @param buf Buffer with data to send
 * @param len Length of data to send
 * @return Number of bytes sent, 0 if error
 */
static size_t uros_transport_internal_write(struct rmw_uros_transport * transport, 
                                  const uint8_t *buf, size_t len)
{
    (void) transport; /* Unused parameter */
    
    /* Use the uros_socket API for writing data */
    int32_t sent_len = uros_send(buf, (uint16_t)len);
    
    /* Return bytes sent or 0 if error */
    return (sent_len > 0) ? (size_t)sent_len : 0;
}

/**
 * @brief Read data from micro-ROS transport
 * 
 * @details Reads data from socket connection if available
 * 
 * @param transport Pointer to rmw_uros_transport structure
 * @param buf Buffer to store received data
 * @param len Maximum number of bytes to read
 * @return Number of bytes read, 0 if no data or error
 */
static size_t uros_transport_internal_read(struct rmw_uros_transport * transport, 
                                 uint8_t *buf, size_t len)
{
    (void) transport; /* Unused parameter */
    
    /* Use uros_socket API for reading data */
    int32_t recv_len = uros_recv(buf, (uint16_t)len);
    
    /* Return bytes read or 0 if no data or error */
    return (recv_len > 0) ? (size_t)recv_len : 0;
}

/* ==========================================================================
 * MICRO-ROS CUSTOM TRANSPORT API IMPLEMENTATION
 * ==========================================================================*/

/**
 * @brief Opens the custom transport for micro-ROS
 * 
 * @details Initializes the socket and connection to the micro-ROS agent.
 *          The agent IP and port can be passed through the transport args.
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @return true if transport was successfully opened, false otherwise
 */
bool uros_transport_open(struct uxrCustomTransport * transport)
{
    /* Extract agent IP and port from transport args if provided */
    uint8_t *args = (uint8_t*)transport->args;
    if (args != NULL && args[0] != 0)
    {
        /* First 4 bytes contain IP address */
        memcpy(agent_info.ip, args, 4);
        
        /* Next 2 bytes contain port number (big-endian) */
        agent_info.port = (args[4] << 8) | args[5];
        
        DEBUG_PRINT("micro-ROS agent set to %d.%d.%d.%d:%d\r\n", 
                    agent_info.ip[0], agent_info.ip[1], agent_info.ip[2], agent_info.ip[3], agent_info.port);
    }
    
    /* Connect to the micro-ROS agent */
    return uros_reconnect(&agent_info);
}

/**
 * @brief Closes the custom transport for micro-ROS
 * 
 * @details Closes the socket connection to the micro-ROS agent
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @return true if transport was successfully closed, false otherwise
 */
bool uros_transport_close(struct uxrCustomTransport * transport)
{
    (void) transport; /* Unused parameter */
    DEBUG_PRINT("micro-ROS transport: Closing connection\r\n");
    return uros_close();
}

/**
 * @brief Writes data through the custom transport for micro-ROS
 * 
 * @details Sends data to the micro-ROS agent with error reporting
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @param buf Buffer with data to send
 * @param len Length of data to send
 * @param err Pointer to error flag (set to 0 on success, 1 on error)
 * @return Number of bytes sent, 0 if error
 */
size_t uros_transport_write(struct uxrCustomTransport* transport, uint8_t * buf, size_t len, uint8_t * err)
{
    (void) transport; /* Unused parameter */
    
    /* Send data using the uros_socket API */
    int32_t sent_len = uros_send(buf, (uint16_t)len);
    
    /* Set error flag based on result */
    if (sent_len > 0)
    {
        *err = 0; /* Success */
        return (size_t)sent_len;
    }
    else
    {
        *err = 1; /* Error */
        return 0;
    }
}

/**
 * @brief Reads data from the custom transport for micro-ROS
 * 
 * @details Receives data from the micro-ROS agent with timeout and error reporting
 * 
 * @param transport Pointer to uxrCustomTransport structure
 * @param buf Buffer to store received data
 * @param len Maximum number of bytes to read
 * @param timeout Maximum time to wait in milliseconds
 * @param err Pointer to error flag (set to 0 on success, 1 on error)
 * @return Number of bytes read, 0 if no data, timeout, or error
 */
size_t uros_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err)
{
    (void) transport; /* Unused parameter */
    
    /* Get start time for timeout calculation */
    uint32_t start_time = osKernelGetTickCount();
    
    /* Poll for data until timeout expires */
    while ((osKernelGetTickCount() - start_time) < (uint32_t)timeout)
    {
        /* Check connection health with heartbeat - reconnect if needed */
        if (!uros_heartbeat(&agent_info))
        {
            DEBUG_PRINT("micro-ROS transport: Connection lost, reconnection failed\r\n");
            *err = 1;
            return 0;
        }
        
        /* Attempt to receive data */
        int32_t recv_len = uros_recv(buf, (uint16_t)len);
        
        if (recv_len > 0)
        {
            /* Data received successfully */
            *err = 0;
            return (size_t)recv_len;
        }
        else if (recv_len < 0)
        {
            /* Error during receive operation */
            *err = 1;
            return 0;
        }
        
        /* Small delay to prevent hammering CPU */
        HAL_Delay(1);
    }
    
    /* Timeout occurred, no data received */
    *err = 1;
    return 0;
}

/* ==========================================================================
 * PUBLIC API IMPLEMENTATION - MICRO-ROS INTEGRATION
 * ==========================================================================*/

/**
 * @brief Initialize the micro-ROS transport with agent information
 * 
 * @details Sets up the connection parameters for the micro-ROS agent
 * 
 * @param agent_ip_addr IP address of the micro-ROS agent (4 bytes)
 * @param agent_port_num Port number of the micro-ROS agent
 * @return true if initialization was successful, false otherwise
 */
bool uros_transport_init(const uint8_t agent_ip_addr[4], uint16_t agent_port_num)
{
    /* Initialize the socket module with CMSIS-RTOS v2 resources */
    if (!uros_socket_init()) {
        DEBUG_PRINT("Failed to initialize micro-ROS socket module\r\n");
        return false;
    }
    
    /* Store connection parameters in the agent_info structure */
    memcpy(agent_info.ip, agent_ip_addr, 4);
    agent_info.port = agent_port_num;
    
    DEBUG_PRINT("micro-ROS transport initialized for agent %d.%d.%d.%d:%d\r\n", 
                agent_info.ip[0], agent_info.ip[1], agent_info.ip[2], agent_info.ip[3], agent_info.port);
                
    return true;
}

/**
 * @brief Provides the transport interface for micro-ROS
 * 
 * @details Creates and returns the transport parameters structure used by micro-ROS.
 *          This is the main entry point for micro-ROS to use the W5500 transport.
 * 
 * @return Transport parameters structure for micro-ROS
 */
rmw_uros_transport_params_t uros_get_transport(void)
{
    rmw_uros_transport_params_t params;
    
    params.framing = MICRO_ROS_FRAMING_NONE;
    params.open_cb = uros_transport_internal_open;
    params.close_cb = uros_transport_internal_close;
    params.write_cb = uros_transport_internal_write;
    params.read_cb = uros_transport_internal_read;
    
    DEBUG_PRINT("micro-ROS transport interface configured\r\n");
                
    return params;
}

#endif /* RMW_UXRCE_TRANSPORT_CUSTOM */