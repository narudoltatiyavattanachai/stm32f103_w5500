/**
 * @file w5500_socket.h
 * @brief Socket API wrapper functions for W5500 Ethernet controller
 * 
 * @details This file provides wrapper functions around the third-party socket
 *          API to isolate application code from direct dependencies on
 *          third-party libraries. All socket operations should use these
 *          wrapper functions instead of calling socket.h functions directly.
 * 
 * @note    This is the foundational socket abstraction layer that all protocol
 *          implementations (ICMP, DNS, DHCP, HTTP, etc.) should use for network
 *          communication.
 * 
 * @see     ip_config.h for network and socket configuration parameters
 */

#ifndef _W5500_SOCKET_H_
#define _W5500_SOCKET_H_

#include <stdint.h>
#include <stdbool.h>
#include "ip_config.h"

/*============================================================================*/
/** @section GENERIC NETWORK INFORMATION
 *  @brief Get current network parameters (IP, subnet, gateway, DNS)
 *  @details These functions provide information about the current network
 *           configuration.
 *============================================================================*/

/**
 * @brief Get current IP address
 * @param ip Pointer to store IP address (4 bytes)
 */
void w5500_get_ip(uint8_t *ip);

/**
 * @brief Get current subnet mask
 * @param subnet Pointer to store subnet mask (4 bytes)
 */
void w5500_get_subnet(uint8_t *subnet);

/**
 * @brief Get current gateway address
 * @param gateway Pointer to store gateway address (4 bytes)
 */
void w5500_get_gateway(uint8_t *gateway);

/**
 * @brief Get current DNS server address
 * @param dns Pointer to store DNS server address (4 bytes)
 */
void w5500_get_dns(uint8_t *dns);

/*============================================================================*/
/** @section GENERIC SOCKET/UDP OPERATIONS
 *  @brief Generic socket, send, receive, and option functions for all services
 *============================================================================*/

/**
 * @brief Create a UDP socket (UDP-only implementation)
 * @param socket_num Socket number to create
 * @param protocol Must be Sn_MR_UDP (UDP only)
 * @param port Local port number
 * @param flag Socket flags
 * @return Socket number if successful, negative value on error
 * @note Only UDP sockets are supported due to resource constraints.
 */
int8_t w5500_socket(uint8_t socket_num, uint8_t protocol, uint16_t port, uint8_t flag);

/**
 * @brief Close a socket
 * @param socket_num Socket number to close
 * @return SOCK_OK if successful, negative value on error
 */
int8_t w5500_close(uint8_t socket_num);

/* TCP listen not supported: UDP-only implementation */
/* int8_t w5500_listen(uint8_t socket_num); */

/**
 * @brief Send data through a UDP socket to a specific destination
 * @param socket_num Socket number to send data on
 * @param data Pointer to data to send
 * @param data_len Length of data to send
 * @param remote_ip IP address of remote host
 * @param remote_port Port number of remote host
 * @return Number of bytes sent if successful, negative value on error
 */
int32_t w5500_sendto(uint8_t socket_num, const uint8_t *data, uint16_t data_len,
                    const uint8_t *remote_ip, uint16_t remote_port);

/**
 * @brief Receive data from a UDP socket
 * @param socket_num Socket number to receive data from
 * @param data Buffer to store received data
 * @param data_len Maximum number of bytes to receive
 * @param remote_ip Buffer to store IP address of sender
 * @param remote_port Pointer to store port number of sender
 * @return Number of bytes received if successful, negative value on error
 */
int32_t w5500_recvfrom(uint8_t socket_num, uint8_t *data, uint16_t data_len,
                      uint8_t *remote_ip, uint16_t *remote_port);

/**
 * @brief Set socket option
 * @param socket_num Socket number to set option for
 * @param option_type Option type
 * @param option_value Pointer to option value
 * @return SOCK_OK if successful, negative value on error
 */
int8_t w5500_setsockopt(uint8_t socket_num, uint8_t option_type, void *option_value);

/**
 * @brief Get socket option
 * @param socket_num Socket number to get option for
 * @param option_type Option type
 * @param option_value Pointer to store option value
 * @return SOCK_OK if successful, negative value on error
 */
int8_t w5500_getsockopt(uint8_t socket_num, uint8_t option_type, void *option_value);

/**
 * @brief Get socket status
 * @param socket_num Socket number to get status for
 * @return Socket status value
 */
uint8_t w5500_socket_status(uint8_t socket_num);

#endif /* _W5500_SOCKET_H_ */
