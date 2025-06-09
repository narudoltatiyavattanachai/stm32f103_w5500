/*============================================================================*/
/*  @file    w5500_dhcp.c                                                     */
/*  @brief   DHCP client implementation for W5500 Ethernet controller         */
/*----------------------------------------------------------------------------*/
/*  This file implements DHCP client functionality for W5500 Ethernet         */
/*  controller, using the WIZnet ioLibrary_Driver. It handles IP address      */
/*  assignment, renewal, and conflict resolution.                             */
/*                                                                            */
/*  Usage:                                                                    */
/*    - Provides network initialization and DHCP client services              */
/*    - All configuration is centralized via ip_config.h macros               */
/*    - Debugging can be enabled with W5500_DEBUG                             */
/*                                                                            */
/*  Maintenance:                                                              */
/*    - Keep all buffer sizes, timeouts, and socket numbers in ip_config.h    */
/*    - Use section headers and Doxygen-style comments for clarity            */
/*============================================================================*/

#include "w5500_dhcp.h"
#include "w5500_socket.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "dhcp.h"
#include <string.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "ip_config.h"

/*============================================================================*/
/*                         DEBUG PRINT MACRO                                  */
/*============================================================================*/
#ifdef W5500_DEBUG
#define DEBUG_PRINT(...)  printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/*============================================================================*/
/*                         PRIVATE CONSTANTS                                  */
/*============================================================================*/
/** @brief Maximum DHCP retry count before timeout */
#define DHCP_MAX_RETRY_COUNT    5

/*============================================================================*/
/*                         PRIVATE STATE AND BUFFERS                          */
/*============================================================================*/
static bool ip_assigned_flag = false;   /**< True if IP is assigned, false otherwise */
static uint8_t dhcp_socket = IP_CONFIG_DHCP_SOCKET; /**< DHCP socket number */

/* Buffer for DHCP client */
static uint8_t dhcp_buffer[IP_CONFIG_DHCP_BUF_SIZE];

/* Cache of DHCP-obtained network parameters */
static uint8_t dhcp_ip[4];       /**< IP address assigned by DHCP */
static uint8_t dhcp_subnet[4];   /**< Subnet mask assigned by DHCP */
static uint8_t dhcp_gateway[4];  /**< Gateway address assigned by DHCP */
static uint8_t dhcp_dns[4];      /**< DNS server address assigned by DHCP */

/* Forward declaration for discovery initialization */
extern bool w5500_discovery_init(void);

/**
 * @brief Initialize network interface using parameters from ip_config.h
 * @return true if initialization and configuration succeeded, false otherwise
 */
bool w5500_network_init(void)
{
    wiz_NetInfo wiznet_info = {0};
    /* Set MAC address from configuration */
    uint8_t mac_addr[] = IP_CONFIG_MAC;
    memcpy(wiznet_info.mac, mac_addr, sizeof(wiznet_info.mac));
    if (IP_CONFIG_USE_DHCP) {
        /* If DHCP is enabled, set zero IPs initially */
        uint8_t zero_ip[4] = {0, 0, 0, 0};
        memcpy(wiznet_info.ip, zero_ip, sizeof(wiznet_info.ip));
        memcpy(wiznet_info.sn, zero_ip, sizeof(wiznet_info.sn));
        memcpy(wiznet_info.gw, zero_ip, sizeof(wiznet_info.gw));
        memcpy(wiznet_info.dns, zero_ip, sizeof(wiznet_info.dns));
        wiznet_info.dhcp = NETINFO_DHCP;
    } else {
        /* Use static IP configuration from ip_config.h */
        uint8_t ip[] = IP_CONFIG_IP;
        uint8_t subnet[] = IP_CONFIG_SUBNET;
        uint8_t gateway[] = IP_CONFIG_GATEWAY;
        uint8_t dns[] = IP_CONFIG_DNS;
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
    ip_assigned_flag = !IP_CONFIG_USE_DHCP;
    DEBUG_PRINT("w5500_network_init: Network initialized, DHCP %s\r\n", IP_CONFIG_USE_DHCP ? "enabled" : "disabled");
    return true;
}

/**
 * @brief Initialize DHCP client process using parameters from ip_config.h
 * @return true if initialization succeeded, false otherwise
 */
bool w5500_dhcp_init(void)
{
    if (!IP_CONFIG_USE_DHCP) {
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

/**
 * @brief Register callbacks for IP assignment events
 * @param ip_assigned Callback for IP assigned event
 * @param ip_changed Callback for IP changed event
 * @param ip_conflict Callback for IP conflict event
 */
void w5500_register_ip_callbacks(void(*ip_assigned)(void), void(*ip_changed)(void), void(*ip_conflict)(void))
{
    /* Register callback functions with DHCP library */
    reg_dhcp_cbfunc(ip_assigned, ip_changed, ip_conflict);
    DEBUG_PRINT("w5500_register_ip_callbacks: Callbacks registered\r\n");
}

/**
 * @brief Process DHCP client tasks, should be called periodically
 * @return Current IP assignment status (IP_STATUS_ASSIGNED, IP_STATUS_CHANGED, ...)
 */
ip_status_t w5500_dhcp_process(void)
{
    if (!IP_CONFIG_USE_DHCP) {
        return ip_assigned_flag ? IP_STATUS_ASSIGNED : IP_STATUS_NONE;
    }
    
    /* Run DHCP client process */
    uint8_t dhcp_status = DHCP_run();
    static bool discovery_started = false;
    ip_status_t result = IP_STATUS_NONE;
    
    /* Map DHCP library status to our status enum */
    switch (dhcp_status) {
        case DHCP_IP_ASSIGN:
            /* Cache network parameters received from DHCP server */
            getIPfromDHCP(dhcp_ip);
            getSNfromDHCP(dhcp_subnet);
            getGWfromDHCP(dhcp_gateway);
            getDNSfromDHCP(dhcp_dns);
            
            ip_assigned_flag = true;
            result = IP_STATUS_ASSIGNED;
            DEBUG_PRINT("w5500_dhcp_process: IP assigned: %d.%d.%d.%d\r\n", 
                       dhcp_ip[0], dhcp_ip[1], dhcp_ip[2], dhcp_ip[3]);
            
            /* Auto-initialize discovery service when IP is assigned */
            if (!discovery_started) {
                if (w5500_discovery_init()) {
                    discovery_started = true;
                    DEBUG_PRINT("w5500_dhcp_process: Discovery service auto-initialized\r\n");
                }
            }
            break;
            
        case DHCP_IP_CHANGED:
            /* Update cached network parameters */
            getIPfromDHCP(dhcp_ip);
            getSNfromDHCP(dhcp_subnet);
            getGWfromDHCP(dhcp_gateway);
            getDNSfromDHCP(dhcp_dns);
            
            ip_assigned_flag = true;
            result = IP_STATUS_CHANGED;
            DEBUG_PRINT("w5500_dhcp_process: IP changed to: %d.%d.%d.%d\r\n", 
                       dhcp_ip[0], dhcp_ip[1], dhcp_ip[2], dhcp_ip[3]);
                       
            /* Restart discovery service when IP changes */
            if (w5500_discovery_init()) {
                discovery_started = true;
                DEBUG_PRINT("w5500_dhcp_process: Discovery service restarted with new IP\r\n");
            }
            break;
            
        case DHCP_FAILED:
            ip_assigned_flag = false;
            result = IP_STATUS_FAILED;
            discovery_started = false;
            DEBUG_PRINT("w5500_dhcp_process: DHCP failed\r\n");
            break;
            
        case DHCP_IP_CONFLICT:
            ip_assigned_flag = false;
            result = IP_STATUS_CONFLICT;
            discovery_started = false;
            DEBUG_PRINT("w5500_dhcp_process: IP conflict detected\r\n");
            break;
            
        case DHCP_RUNNING:
            result = IP_STATUS_PROCESSING;
            break;
            
        case DHCP_STOPPED:
            result = IP_STATUS_STOPPED;
            break;
            
        case DHCP_IP_LEASED:
            /* Nothing to do, already leased */
            result = IP_STATUS_ASSIGNED;
            break;
            
        default:
            /* No change in status */
            break;
    }
    
    return result;
}

/**
 * @brief Check if an IP address is assigned
 * @return true if IP is assigned, false otherwise
 */
bool w5500_is_ip_assigned(void)
{
    return ip_assigned_flag;
}

/**
 * @brief Get the current IP address obtained via DHCP
 * @param ip Buffer to store the IP address (4 bytes)
 */
void w5500_dhcp_get_ip(uint8_t* ip)
{
    if (IP_CONFIG_USE_DHCP && ip_assigned_flag) {
        memcpy(ip, dhcp_ip, 4);
    } else {
        uint8_t config_ip[] = IP_CONFIG_IP;
        memcpy(ip, config_ip, 4);
    }
}

/**
 * @brief Get the current subnet mask obtained via DHCP
 * @param subnet Buffer to store the subnet mask (4 bytes)
 */
void w5500_dhcp_get_subnet(uint8_t* subnet)
{
    if (IP_CONFIG_USE_DHCP && ip_assigned_flag) {
        memcpy(subnet, dhcp_subnet, 4);
    } else {
        uint8_t config_subnet[] = IP_CONFIG_SUBNET;
        memcpy(subnet, config_subnet, 4);
    }
}

/**
 * @brief Get the current gateway address obtained via DHCP
 * @param gateway Buffer to store the gateway address (4 bytes)
 */
void w5500_dhcp_get_gateway(uint8_t* gateway)
{
    if (IP_CONFIG_USE_DHCP && ip_assigned_flag) {
        memcpy(gateway, dhcp_gateway, 4);
    } else {
        uint8_t config_gateway[] = IP_CONFIG_GATEWAY;
        memcpy(gateway, config_gateway, 4);
    }
}

/**
 * @brief Get the current DNS server address obtained via DHCP
 * @param dns Buffer to store the DNS server address (4 bytes)
 */
void w5500_dhcp_get_dns(uint8_t* dns)
{
    if (IP_CONFIG_USE_DHCP && ip_assigned_flag) {
        memcpy(dns, dhcp_dns, 4);
    } else {
        uint8_t config_dns[] = IP_CONFIG_DNS;
        memcpy(dns, config_dns, 4);
    }
}

/**
 * @brief Handle DHCP 1-second timer
 */
void w5500_dhcp_time_handler(void)
{
    /* Call DHCP library's time handler */
    if (IP_CONFIG_USE_DHCP) {
        DHCP_time_handler();
    }
}
