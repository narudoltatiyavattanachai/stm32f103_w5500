/*============================================================================*/
/*  @file    w5500_socket.c                                                   */
/*  @brief   Socket API wrapper implementation for W5500 Ethernet controller   */
/*----------------------------------------------------------------------------*/
/*  This file implements wrapper functions around the third-party socket API   */
/*  (WIZnet ioLibrary_Driver) to isolate application code from direct          */
/*  dependencies on third-party libraries. All socket operations should use    */
/*  these wrappers instead of calling socket.h functions directly.             */
/*                                                                            */
/*  Usage:                                                                    */
/*    - Provides a stable, testable, and maintainable socket abstraction.      */
/*    - All configuration is centralized via eth_config.h macros.               */
/*    - Debugging can be enabled with W5500_DEBUG.                             */
/*                                                                            */
/*  Maintenance:                                                              */
/*    - Keep all buffer sizes, timeouts, and socket numbers in eth_config.h.    */
/*    - Use section headers and Doxygen-style comments for clarity.            */
/*============================================================================*/

#include "w5500_socket.h"
#include "socket.h"
#include "wizchip_conf.h"
#include <string.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"

/*============================================================================*/
/*                         DEBUG PRINT MACRO                                  */
/*============================================================================*/
#ifdef W5500_DEBUG
#define DEBUG_PRINT(...)  printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/*============================================================================*/
/** @section GENERIC NETWORK INFORMATION
 *  @brief Network parameter getters (IP, subnet, gateway, DNS)
 *============================================================================*/

/**
 * @brief Initialize device discovery service
 * @return true if initialization succeeded, false otherwise
 */
/*============================================================================*/
/**
 * @brief Get current IP address
 * @param ip Pointer to buffer to store IP address (4 bytes)
 */
/*============================================================================*/
void w5500_get_ip(uint8_t *ip)
{
    if (ip != NULL) {
        getSIPR(ip);
    }
}

/*============================================================================*/
/**
 * @brief Get current subnet mask
 * @param subnet Pointer to buffer to store subnet mask (4 bytes)
 */
/*============================================================================*/
void w5500_get_subnet(uint8_t *subnet)
{
    if (subnet != NULL) {
        getSUBR(subnet);
    }
}

/*============================================================================*/
/**
 * @brief Get current gateway address
 * @param gateway Pointer to buffer to store gateway address (4 bytes)
 */
/*============================================================================*/
void w5500_get_gateway(uint8_t *gateway)
{
    if (gateway != NULL) {
        getGAR(gateway);
    }
}

/*============================================================================*/
/**
 * @brief Get current DNS server address
 * @param dns Pointer to buffer to store DNS server address (4 bytes)
 */
/*============================================================================*/
void w5500_get_dns(uint8_t *dns)
{
    if (dns != NULL) {
        getDNSR(dns);
    }
}

/*============================================================================*/
/** @section GENERIC SOCKET/UDP OPERATIONS
 *  @brief Core UDP socket functionality
 *============================================================================*/

/*============================================================================*/
/**
 * @brief Create a UDP socket (UDP-only implementation)
 * @param socket_num Socket number to create
 * @param protocol Must be Sn_MR_UDP (UDP only)
 * @param port Local port number
 * @param flag Socket flags
 * @return Socket number if successful, negative value on error
 */
/*============================================================================*/
int8_t w5500_socket(uint8_t socket_num, uint8_t protocol, uint16_t port, uint8_t flag)
{
    /* Only UDP is supported in this implementation */
    if (protocol != Sn_MR_UDP) {
        DEBUG_PRINT("w5500_socket: Only UDP protocol supported\r\n");
        return SOCK_ERROR;
    }
    
    /* Close the socket first if it was already open */
    w5500_close(socket_num);
    
    /* Create the socket using third-party library */
    int8_t ret = socket(socket_num, protocol, port, flag);
    
    if (ret == socket_num) {
        DEBUG_PRINT("w5500_socket: Created UDP socket %d on port %d\r\n", socket_num, port);
    } else {
        DEBUG_PRINT("w5500_socket: Failed to create socket %d, error %d\r\n", socket_num, ret);
    }
    
    return ret;
}

/*============================================================================*/
/**
 * @brief Close a socket
 * @param socket_num Socket number to close
 * @return SOCK_OK if successful, negative value on error
 */
/*============================================================================*/
int8_t w5500_close(uint8_t socket_num)
{
    int8_t ret = close(socket_num);
    DEBUG_PRINT("w5500_close: Closing socket %d, result %d\r\n", socket_num, ret);
    return ret;
}

/*============================================================================*/
/**
 * @brief Send data through a UDP socket to a specific destination
 * @param socket_num Socket number to send data on
 * @param data Pointer to data to send
 * @param data_len Length of data to send
 * @param remote_ip IP address of remote host
 * @param remote_port Port number of remote host
 * @return Number of bytes sent if successful, negative value on error
 */
/*============================================================================*/
int32_t w5500_sendto(uint8_t socket_num, const uint8_t *data, uint16_t data_len,
                     const uint8_t *remote_ip, uint16_t remote_port)
{
    int32_t ret = sendto(socket_num, (uint8_t*)data, data_len, (uint8_t*)remote_ip, remote_port);
    
    if (ret > 0) {
        DEBUG_PRINT("w5500_sendto: Sent %d bytes on socket %d to %d.%d.%d.%d:%d\r\n", 
                   ret, socket_num, remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
    } else {
        DEBUG_PRINT("w5500_sendto: Failed to send on socket %d, error %d\r\n", socket_num, ret);
    }
    
    return ret;
}

/*============================================================================*/
/**
 * @brief Receive data from a UDP socket
 * @param socket_num Socket number to receive data from
 * @param data Buffer to store received data
 * @param data_len Maximum number of bytes to receive
 * @param remote_ip Buffer to store IP address of sender
 * @param remote_port Pointer to store port number of sender
 * @return Number of bytes received if successful, negative value on error
 */
/*============================================================================*/
int32_t w5500_recvfrom(uint8_t socket_num, uint8_t *data, uint16_t data_len, 
                      uint8_t *remote_ip, uint16_t *remote_port)
{
    int32_t ret = recvfrom(socket_num, data, data_len, remote_ip, remote_port);
    
    if (ret > 0) {
        DEBUG_PRINT("w5500_recvfrom: Received %d bytes on socket %d from %d.%d.%d.%d:%d\r\n", 
                   ret, socket_num, remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], *remote_port);
    }
    
    return ret;
}

/*============================================================================*/
/**
 * @brief Set socket option
 * @param socket_num Socket number
 * @param option_type Option type
 * @param option_value Pointer to option value
 * @return 0 on success, negative value on error
 */
/*============================================================================*/
int8_t w5500_setsockopt(uint8_t socket_num, uint8_t option_type, void *option_value)
{
    DEBUG_PRINT("w5500_setsockopt: Setting option %d on socket %d\r\n", option_type, socket_num);
    return setsockopt(socket_num, option_type, option_value);
}

/*============================================================================*/
/**
 * @brief Get socket option
 * @param socket_num Socket number
 * @param option_type Option type
 * @param option_value Pointer to option value
 * @return 0 on success, negative value on error
 */
/*============================================================================*/
int8_t w5500_getsockopt(uint8_t socket_num, uint8_t option_type, void *option_value)
{
    int8_t ret = getsockopt(socket_num, option_type, option_value);
    DEBUG_PRINT("w5500_getsockopt: Getting option %d from socket %d, result %d\r\n", 
                option_type, socket_num, ret);
    return ret;
}

/*============================================================================*/
/**
 * @brief Get socket status
 * @param socket_num Socket number
 * @return Socket status register value
 */
/*============================================================================*/
uint8_t w5500_socket_status(uint8_t socket_num)
{
    uint8_t status = getSn_SR(socket_num);
    DEBUG_PRINT("w5500_socket_status: Socket %d status = 0x%02X\r\n", socket_num, status);
    return status;
}



/**
 * @brief Create a UDP socket (UDP-only implementation)
 * @param socket_num Socket number
 * @param protocol Must be Sn_MR_UDP (UDP only)
 * @param port Port number
 * @param flag Socket flag (0 for normal operation)
 * @return Socket number on success, negative value on error
 * @note Only UDP sockets are supported due to resource constraints.
 */
int8_t w5500_socket(uint8_t socket_num, uint8_t protocol, uint16_t port, uint8_t flag) { ... }

/**
 * @brief Close a socket
 * @param socket_num Socket number
 * @return 0 on success, negative value on error
 */
int8_t w5500_close(uint8_t socket_num) { ... }

/**
 * @brief Send data through a UDP socket to a specific destination
 * @param socket_num Socket number
 * @param data Pointer to data buffer
 * @param data_len Length of data to send
 * @param remote_ip Pointer to remote IP address (4 bytes)
 * @param remote_port Remote port number
 * @return Number of bytes sent on success, negative value on error
 */
int32_t w5500_sendto(uint8_t socket_num, const uint8_t *data, uint16_t data_len, const uint8_t *remote_ip, uint16_t remote_port) { ... }

/**
 * @brief Receive data from a UDP socket
 * @param socket_num Socket number
 * @param data Pointer to buffer to store received data
 * @param data_len Length of buffer
 * @param remote_ip Pointer to buffer to store remote IP address (4 bytes)
 * @param remote_port Pointer to store remote port number
 * @return Number of bytes received on success, negative value on error
 */
int32_t w5500_recvfrom(uint8_t socket_num, uint8_t *data, uint16_t data_len, uint8_t *remote_ip, uint16_t *remote_port) { ... }

/**
 * @brief Set socket option
 * @param socket_num Socket number
 * @param option_type Option type
 * @param option_value Pointer to option value
 * @return 0 on success, negative value on error
 */
int8_t w5500_setsockopt(uint8_t socket_num, uint8_t option_type, void *option_value) { ... }

/**
 * @brief Get socket option
 * @param socket_num Socket number
 * @param option_type Option type
 * @param option_value Pointer to option value
 * @return 0 on success, negative value on error
 */
int8_t w5500_getsockopt(uint8_t socket_num, uint8_t option_type, void *option_value) { ... }

/**
 * @brief Get socket status
 * @param socket_num Socket number
 * @return Socket status register value
 */
uint8_t w5500_socket_status(uint8_t socket_num) { ... }


/*============================================================================*/
 /**
  * @brief Initialize network interface using parameters from eth_config.h
  * @return true if initialization and configuration succeeded, false otherwise
  */
/*============================================================================*/
 bool w5500_network_init(void)
 {
     wiz_NetInfo wiznet_info = {0};
     
     /* Set MAC address from configuration */
     uint8_t mac_addr[] = ETH_CONFIG_MAC;
     memcpy(wiznet_info.mac, mac_addr, sizeof(wiznet_info.mac));
    
    if (ETH_CONFIG_USE_DHCP) {
         /* If DHCP is enabled, set zero IPs initially */
         uint8_t zero_ip[4] = {0, 0, 0, 0};
         memcpy(wiznet_info.ip, zero_ip, sizeof(wiznet_info.ip));
        memcpy(wiznet_info.sn, zero_ip, sizeof(wiznet_info.sn));
        memcpy(wiznet_info.gw, zero_ip, sizeof(wiznet_info.gw));
        memcpy(wiznet_info.dns, zero_ip, sizeof(wiznet_info.dns));
         wiznet_info.dhcp = NETINFO_DHCP;
     } else {
         /* Use static IP configuration from eth_config.h */
         uint8_t ip[] = ETH_CONFIG_IP;
         uint8_t subnet[] = ETH_CONFIG_SUBNET;
         uint8_t gateway[] = ETH_CONFIG_GATEWAY;
         uint8_t dns[] = ETH_CONFIG_DNS;
         
         memcpy(wiznet_info.ip, ip, sizeof(wiznet_info.ip));
        memcpy(wiznet_info.sn, subnet, sizeof(wiznet_info.sn));
        memcpy(wiznet_info.gw, gateway, sizeof(wiznet_info.gw));
        memcpy(wiznet_info.dns, dns, sizeof(wiznet_info.dns));
         wiznet_info.dhcp = NETINFO_STATIC;
     }
     
    /* Apply network configuration to W5500 */
    wizchip_setnetinfo(&wiznet_info);
    wiz_NetInfo verify_info;
    wizchip_getnetinfo(&verify_info);
    if (memcmp(&wiznet_info, &verify_info, sizeof(wiz_NetInfo)) != 0) {
        DEBUG_PRINT("w5500_network_init: Failed to set network info!\r\n");
        return false;
    }
    /* If not using DHCP, IP is considered assigned */
    ip_assigned_flag = !ETH_CONFIG_USE_DHCP;
    DEBUG_PRINT("w5500_network_init: Network initialized, DHCP %s\r\n", 
                ETH_CONFIG_USE_DHCP ? "enabled" : "disabled");
    return true;
 }




/*============================================================================*/
 /**
  * @brief Initialize DHCP client process using parameters from eth_config.h
  * @return true if initialization succeeded, false otherwise
  */
/*============================================================================*/
 bool w5500_dhcp_init(void)
 {
     if (!ETH_CONFIG_USE_DHCP) {
         DEBUG_PRINT("w5500_dhcp_init: DHCP not enabled in configuration\r\n");
         return false;
     }
     
     /* Initialize DHCP client with socket and buffer */
     DHCP_init(dhcp_socket, dhcp_buffer);
     
     /* Reset IP assignment flag */
     ip_assigned_flag = false;
     
     DEBUG_PRINT("w5500_dhcp_init: DHCP initialized on socket %d\r\n", dhcp_socket);
     
     return true;
 }



/*============================================================================*/
/**
 * @brief Register callbacks for IP assignment events
 * @param ip_assigned Callback for IP assigned event
 * @param ip_changed Callback for IP changed event
 * @param ip_conflict Callback for IP conflict event
 */
/*============================================================================*/
void w5500_register_ip_callbacks(void(*ip_assigned)(void), 
                                void(*ip_changed)(void), 
                                void(*ip_conflict)(void))
 {
     /* Register callback functions with DHCP library */
     reg_dhcp_cbfunc(ip_assigned, ip_changed, ip_conflict);
     
     DEBUG_PRINT("w5500_register_ip_callbacks: Callbacks registered\r\n");
 }



/*============================================================================*/
/**
 * @brief Process DHCP client tasks, should be called periodically
 * @return Current IP assignment status (W5500_IP_ASSIGNED, W5500_IP_CHANGED, W5500_IP_CONFLICT, or W5500_IP_NONE)
 */
/*============================================================================*/
w5500_ip_status_t w5500_dhcp_process(void)
 {
     if (!ETH_CONFIG_USE_DHCP) {
         return ip_assigned_flag ? W5500_IP_ASSIGNED : W5500_IP_NONE;
     }
     
     /* Run DHCP client process */
     uint8_t dhcp_status = DHCP_run();
     w5500_ip_status_t result = W5500_IP_NONE;
     static bool discovery_started = false;
     
     /* Map DHCP library status to our status enum */
     switch (dhcp_status) {
         case DHCP_IP_ASSIGN:
             ip_assigned_flag = true;
             result = W5500_IP_ASSIGNED;
             
             DEBUG_PRINT("w5500_dhcp_process: IP assigned\r\n");
             
             /* Auto-initialize discovery service when IP is assigned */
             if (!discovery_started) {
                 if (w5500_discovery_init()) {
                     discovery_started = true;
                     DEBUG_PRINT("w5500_dhcp_process: Discovery service auto-initialized\r\n");
                 }
             }
             break;
             
         case DHCP_IP_CHANGED:
             ip_assigned_flag = true;
             result = W5500_IP_CHANGED;
             
             DEBUG_PRINT("w5500_dhcp_process: IP changed\r\n");
             
             /* Restart discovery service when IP changes */
             if (w5500_discovery_init()) {
                 discovery_started = true;
                 DEBUG_PRINT("w5500_dhcp_process: Discovery service restarted with new IP\r\n");
             }
             break;
             
         case DHCP_IP_CONFLICT:
             ip_assigned_flag = false;
             result = W5500_IP_CONFLICT;
             discovery_started = false;
             DEBUG_PRINT("w5500_dhcp_process: IP conflict detected\r\n");
             break;
             
         default:
             /* No change in status */
             break;
     }
     
     return result;
 }




/*============================================================================*/
/**
 * @brief Check if an IP address is assigned
 * @return true if IP is assigned, false otherwise
 */
/*============================================================================*/
bool w5500_is_ip_assigned(void)
 {
     return ip_assigned_flag;
 }



/*============================================================================*/
/**
 * @brief Handle DHCP 1-second timer
 */
/*============================================================================*/
void w5500_dhcp_time_handler(void)
 {
     /* Call DHCP library's time handler */
     if (ETH_CONFIG_USE_DHCP) {
         DHCP_time_handler();
     }
 }




/*============================================================================*/
/**
 * @brief Get current IP address
 * @param ip Pointer to buffer to store IP address (4 bytes)
 */
/*============================================================================*/
void w5500_get_ip(uint8_t *ip)
 {
     if (ip != NULL) {
         getSIPR(ip);
     }
 }




/*============================================================================*/
/**
 * @brief Get current subnet mask
 * @param subnet Pointer to buffer to store subnet mask (4 bytes)
 */
/*============================================================================*/
void w5500_get_subnet(uint8_t *subnet)
 {
     if (subnet != NULL) {
         getSUBR(subnet);
     }
 }



/*============================================================================*/
/**
 * @brief Get current gateway address
 * @param gateway Pointer to buffer to store gateway address (4 bytes)
 */
/*============================================================================*/
void w5500_get_gateway(uint8_t *gateway)
 {
     if (gateway != NULL) {
         getGAR(gateway);
     }
 }



/*============================================================================*/
/**
 * @brief Get current DNS server address
 * @param dns Pointer to buffer to store DNS server address (4 bytes)
 */
/*============================================================================*/
void w5500_get_dns(uint8_t *dns)
{
    if (dns != NULL) {
        getDNSR(dns);
    }
}

/*============================================================================*/
/**
 * @brief Initialize device discovery service
 * @return true if initialization succeeded, false otherwise
 */
/*============================================================================*/
bool w5500_discovery_init(void)
{
    /* Check if IP is assigned - discovery won't work without IP */
    if (!w5500_is_ip_assigned()) {
        DEBUG_PRINT("w5500_discovery_init: No IP assigned, cannot initialize discovery\r\n");
        return false;
    }
    
    /* Close the socket if it was already in use */
    w5500_close(discovery_socket);
    
    /* Open a UDP socket for discovery */
    if (w5500_socket(discovery_socket, Sn_MR_UDP, ETH_CONFIG_DISCOVERY_PORT, 0) != discovery_socket) {
        DEBUG_PRINT("w5500_discovery_init: Failed to create discovery socket\r\n");
        return false;
    }
    
    discovery_initialized = true;
    DEBUG_PRINT("w5500_discovery_init: Device discovery initialized on socket %d port %d\r\n", 
              discovery_socket, ETH_CONFIG_DISCOVERY_PORT);
    
    /* Send initial announcement */
    w5500_send_announcement();
    
    return true;
}




/*============================================================================*/
/**
 * @brief Process device discovery tasks, handle incoming discovery requests
 * @return true if any activity occurred, false otherwise
 */
/*============================================================================*/
bool w5500_discovery_process(void)
{
    if (!discovery_initialized || !ip_assigned_flag) {
        return false;
    }
    
    uint8_t remote_ip[4];
    uint16_t remote_port;
    int32_t len;
    bool activity = false;
    
    /* Check for incoming discovery requests */
    if ((len = w5500_recvfrom(discovery_socket, discovery_buffer, sizeof(discovery_buffer), 
                            remote_ip, &remote_port)) > 0) {
        activity = true;
        
        /* Parse discovery request */
        if (strncmp((char*)discovery_buffer, "DISCOVER_STM32", 14) == 0) {
            DEBUG_PRINT("w5500_discovery_process: Received discovery request from %d.%d.%d.%d:%d\r\n",
                      remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
            
            /* Prepare response with device information */
            uint8_t local_ip[4];
            w5500_get_ip(local_ip);
            
            /* Format device info as JSON */
            len = snprintf((char*)discovery_resp_buffer, sizeof(discovery_resp_buffer),
                         "{\"hostname\":\"%s\",\"ip\":\"%d.%d.%d.%d\",\"type\":\"%s\",\"version\":\"%s\"}",
                         ETH_CONFIG_HOSTNAME, 
                         local_ip[0], local_ip[1], local_ip[2], local_ip[3],
                         ETH_CONFIG_DEVICE_TYPE, 
                         ETH_CONFIG_FW_VERSION);
            
            /* Send response */
            w5500_sendto(discovery_socket, discovery_resp_buffer, len, remote_ip, remote_port);
            
            /* Call discovery callback if registered */
            if (discovery_callback != NULL) {
                discovery_callback(remote_ip, remote_port);
            }
        }
    }
    
    return activity;
}



/*============================================================================*/
/**
 * @brief Send device announcement broadcast
 * @return true if announcement was sent successfully, false otherwise
 */
/*============================================================================*/
bool w5500_send_announcement(void)
{
    if (!discovery_initialized || !ip_assigned_flag) {
        return false;
    }
    
    /* Prepare broadcast address */
    uint8_t subnet[4];
    uint8_t local_ip[4];
    uint8_t broadcast_ip[4];
    w5500_get_subnet(subnet);
    w5500_get_ip(local_ip);
    
    /* Calculate broadcast address (IP | ~Subnet) */
    for (int i = 0; i < 4; i++) {
        broadcast_ip[i] = local_ip[i] | (~subnet[i] & 0xFF);
    }
    
    /* Format announcement message as JSON */
    int32_t len = snprintf((char*)discovery_resp_buffer, sizeof(discovery_resp_buffer),
                        "{\"announce\":\"stm32_device\",\"hostname\":\"%s\",\"ip\":\"%d.%d.%d.%d\",\"type\":\"%s\"}",
                        ETH_CONFIG_HOSTNAME, 
                        local_ip[0], local_ip[1], local_ip[2], local_ip[3],
                        ETH_CONFIG_DEVICE_TYPE);
    
    /* Send broadcast */
    if (w5500_sendto(discovery_socket, discovery_resp_buffer, len, broadcast_ip, ETH_CONFIG_DISCOVERY_PORT) <= 0) {
        DEBUG_PRINT("w5500_send_announcement: Failed to send announcement\r\n");
        return false;
    }
    
    DEBUG_PRINT("w5500_send_announcement: Broadcast announcement to %d.%d.%d.%d:%d\r\n",
              broadcast_ip[0], broadcast_ip[1], broadcast_ip[2], broadcast_ip[3], ETH_CONFIG_DISCOVERY_PORT);
    
    return true;
}

/*============================================================================*/
/**
 * @brief Set callback for when the device is discovered
 * @param callback Callback function to call when device is discovered
 */
/*============================================================================*/
void w5500_set_discovery_callback(void (*callback)(uint8_t *remote_ip, uint16_t remote_port))
{
    discovery_callback = callback;
}

/*============================================================================*/
/**
 * @brief Create a UDP socket (UDP-only implementation)
 * @param socket_num Socket number
 * @param protocol Must be Sn_MR_UDP (UDP only)
 * @param port Port number
 * @param flag Socket flag (0 for normal operation)
 * @return Socket number on success, negative value on error
 * @note Only UDP sockets are supported due to resource constraints.
 */
/*============================================================================*/
int8_t w5500_socket(uint8_t socket_num, uint8_t protocol, uint16_t port, uint8_t flag)
{
    if (protocol != Sn_MR_UDP) {
        DEBUG_PRINT("w5500_socket: Only UDP sockets are supported!\r\n");
        return -1;
    }
    DEBUG_PRINT("w5500_socket: Creating UDP socket %d port %d flag %d\r\n", socket_num, port, flag);
    return socket(socket_num, protocol, port, flag);
}

/*============================================================================*/
/*                         SOCKET STATE AND DATA OPERATIONS                   */
/*============================================================================*/

/*============================================================================*/
/**
 * @brief Close a socket
 * @param socket_num Socket number
 * @return 0 on success, negative value on error
 */
/*============================================================================*/
int8_t w5500_close(uint8_t socket_num)
{
     DEBUG_PRINT("w5500_close: Closing socket %d\r\n", socket_num);
     return close(socket_num);
 }

/*============================================================================*/
/**
 * @brief Send data through a UDP socket to a specific destination
 * @param socket_num Socket number
 * @param data Pointer to data buffer
 * @param data_len Length of data to send
 * @param remote_ip Pointer to remote IP address (4 bytes)
 * @param remote_port Remote port number
 * @return Number of bytes sent on success, negative value on error
 */
/*============================================================================*/
int32_t w5500_sendto(uint8_t socket_num, const uint8_t *data, uint16_t data_len, 
                    const uint8_t *remote_ip, uint16_t remote_port)
{
    if (!data || data_len == 0 || !remote_ip) {
        DEBUG_PRINT("w5500_sendto: Invalid input (null pointer or zero length)\r\n");
        return -1;
    }
    DEBUG_PRINT("w5500_sendto: Sending %d bytes on socket %d to %d.%d.%d.%d:%d\r\n", 
                data_len, socket_num, remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
    /* Cast away const qualifiers as third-party API doesn't use const, but these parameters won't be modified */
    int32_t result = sendto(socket_num, (uint8_t*)data, data_len, (uint8_t*)remote_ip, remote_port);
    if (result < 0) {
        DEBUG_PRINT("w5500_sendto: sendto() failed with code %ld\r\n", result);
    }
    return result;
}

/*============================================================================*/
/**
 * @brief Receive data from a UDP socket
 * @param socket_num Socket number
 * @param data Pointer to buffer to store received data
 * @param data_len Length of buffer
 * @param remote_ip Pointer to buffer to store remote IP address (4 bytes)
 * @param remote_port Pointer to store remote port number
 * @return Number of bytes received on success, negative value on error
 */
/*============================================================================*/
int32_t w5500_recvfrom(uint8_t socket_num, uint8_t *data, uint16_t data_len, 
                      uint8_t *remote_ip, uint16_t *remote_port)
{
    int32_t ret = recvfrom(socket_num, data, data_len, remote_ip, remote_port);
    if (ret > 0) {
        DEBUG_PRINT("w5500_recvfrom: Received %d bytes on socket %d from %d.%d.%d.%d:%d\r\n", 
                   ret, socket_num, remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], *remote_port);
    }
    return ret;
}

/*============================================================================*/
/**
 * @brief Set socket option
 * @param socket_num Socket number
 * @param option_type Option type
 * @param option_value Pointer to option value
 * @return 0 on success, negative value on error
 */
/*============================================================================*/
int8_t w5500_setsockopt(uint8_t socket_num, uint8_t option_type, void *option_value)
{
    DEBUG_PRINT("w5500_setsockopt: Setting option %d on socket %d\r\n", option_type, socket_num);
    return setsockopt(socket_num, option_type, option_value);
}

/*============================================================================*/
/**
 * @brief Get socket option
 * @param socket_num Socket number
 * @param option_type Option type
 * @param option_value Pointer to option value
 * @return 0 on success, negative value on error
 */
/*============================================================================*/
int8_t w5500_getsockopt(uint8_t socket_num, uint8_t option_type, void *option_value)
{
    int8_t ret = getsockopt(socket_num, option_type, option_value);
    DEBUG_PRINT("w5500_getsockopt: Getting option %d from socket %d, result %d\r\n", 
                option_type, socket_num, ret);
    return ret;
}

/*============================================================================*/
/**
 * @brief Get socket status
 * @param socket_num Socket number
 * @return Socket status register value
 */
/*============================================================================*/
uint8_t w5500_socket_status(uint8_t socket_num)
{
    uint8_t status = getSn_SR(socket_num);
    DEBUG_PRINT("w5500_socket_status: Socket %d status = 0x%02X\r\n", socket_num, status);
    return status;
}