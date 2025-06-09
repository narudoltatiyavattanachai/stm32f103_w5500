/**
 * @file w5500_socket.h
 * @brief Socket API wrapper functions for W5500 Ethernet controller
 * 
 * @details This file provides wrapper functions around the third-party socket
 *          API to isolate application code from direct dependencies on
 *          third-party libraries. All socket operations should use these
 *          wrapper functions instead of calling socket.h functions directly.
 */

#ifndef _W5500_SOCKET_H_
#define _W5500_SOCKET_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Create a socket with the specified parameters
 * @param socket_num Socket number to create
 * @param protocol Protocol type (TCP, UDP, etc.)
 * @param port Local port number
 * @param flag Socket flags
 * @return Socket number if successful, negative value on error
 */
int8_t w5500_socket(uint8_t socket_num, uint8_t protocol, uint16_t port, uint8_t flag);

/**
 * @brief Close a socket
 * @param socket_num Socket number to close
 * @return SOCK_OK if successful, negative value on error
 */
int8_t w5500_close(uint8_t socket_num);

/**
 * @brief Listen for incoming connections on a socket
 * @param socket_num Socket number to listen on
 * @return SOCK_OK if successful, negative value on error
 */
int8_t w5500_listen(uint8_t socket_num);

/**
 * @brief Connect to a remote host
 * @param socket_num Socket number to use for connection
 * @param remote_ip IP address of remote host
 * @param remote_port Port number of remote host
 * @return SOCK_OK if successful, negative value on error
 */
int8_t w5500_connect(uint8_t socket_num, const uint8_t *remote_ip, uint16_t remote_port);

/**
 * @brief Disconnect a TCP connection
 * @param socket_num Socket number to disconnect
 * @return SOCK_OK if successful, negative value on error
 */
int8_t w5500_disconnect(uint8_t socket_num);

/**
 * @brief Send data through a connected TCP socket
 * @param socket_num Socket number to send data on
 * @param data Pointer to data to send
 * @param data_len Length of data to send
 * @return Number of bytes sent if successful, negative value on error
 */
int32_t w5500_send(uint8_t socket_num, const uint8_t *data, uint16_t data_len);

/**
 * @brief Receive data from a connected TCP socket
 * @param socket_num Socket number to receive data from
 * @param data Buffer to store received data
 * @param data_len Maximum number of bytes to receive
 * @return Number of bytes received if successful, negative value on error
 */
int32_t w5500_recv(uint8_t socket_num, uint8_t *data, uint16_t data_len);

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


#endif /* _W5500_SOCKET_H */
