/**
 * @file w5500_dhcp.h
 * @brief DHCP client implementation for W5500 Ethernet controller
 * 
 * @details This file provides DHCP client functionality to dynamically obtain
 *          network configuration parameters (IP, subnet, gateway, DNS). It uses
 *          the W5500 socket API for all network operations.
 * 
 * @see     ip_config.h for network and socket configuration parameters
 */

#ifndef _W5500_DHCP_H_
#define _W5500_DHCP_H_

#include <stdint.h>
#include <stdbool.h>
#include "ip_config.h"

/* Using centralized IP status type from ip_config.h */

/*============================================================================*/
/** @section DHCP CLIENT SERVICE (Socket 0)
 *  @brief DHCP initialization, periodic handler, and IP assignment
 *  @details All DHCP-related logic and state management for dynamic IP
 *           assignment. Socket defined by IP_CONFIG_DHCP_SOCKET.
 *============================================================================*/

/**
 * @brief Initialize network interface using parameters from ip_config.h
 * @return true if successful, false otherwise
 */
bool w5500_network_init(void);

/**
 * @brief Initialize DHCP client process using parameters from ip_config.h
 * @return true if successful, false otherwise
 */
bool w5500_dhcp_init(void);

/**
 * @brief Register callbacks for IP assignment events
 * @param ip_assigned Callback function for successful IP assignment
 * @param ip_changed Callback function for IP address change
 * @param ip_conflict Callback function for IP address conflict
 */
void w5500_register_ip_callbacks(void(*ip_assigned)(void), 
                               void(*ip_changed)(void), 
                               void(*ip_conflict)(void));

/**
 * @brief Process DHCP client tasks, should be called periodically
 * @return Current IP assignment status
 */
ip_status_t w5500_dhcp_process(void);

/**
 * @brief Check if an IP address is assigned
 * @return true if IP is assigned, false otherwise
 */
bool w5500_is_ip_assigned(void);

/**
 * @brief Get the current IP address obtained via DHCP
 * @param ip Buffer to store the IP address (4 bytes)
 */
void w5500_dhcp_get_ip(uint8_t* ip);

/**
 * @brief Get the current subnet mask obtained via DHCP
 * @param subnet Buffer to store the subnet mask (4 bytes)
 */
void w5500_dhcp_get_subnet(uint8_t* subnet);

/**
 * @brief Get the current gateway address obtained via DHCP
 * @param gateway Buffer to store the gateway address (4 bytes)
 */
void w5500_dhcp_get_gateway(uint8_t* gateway);

/**
 * @brief Get the current DNS server address obtained via DHCP
 * @param dns Buffer to store the DNS server address (4 bytes)
 */
void w5500_dhcp_get_dns(uint8_t* dns);

/**
 * @brief Handle DHCP 1-second timer
 * @details Should be called once per second for proper DHCP operation
 */
void w5500_dhcp_time_handler(void);

#endif /* _W5500_DHCP_H_ */
