/**
 * @file    w5500_socket.h
 * @brief   Socket API wrapper for W5500 Ethernet controller
 * @author  Narudol T.
 * @date    2025-06-10
 * 
 * @details This file provides wrapper functions around the WIZnet socket API
 *          to simplify application code and create a consistent interface.
 *          The socket API is designed to be protocol-agnostic where possible,
 *          with specialized functions for UDP and TCP operations.
 */

#ifndef W5500_SOCKET_H
#define W5500_SOCKET_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Socket type enumeration
 */
typedef enum {
    W5500_SOCK_TCP = 0,  /**< TCP socket type */
    W5500_SOCK_UDP = 1   /**< UDP socket type */
} w5500_sock_type_t;

/**
 * @brief Error codes for socket operations
 */
typedef enum {
    W5500_SOCK_OK = 0,          /**< Operation successful */
    W5500_SOCK_ERROR = -1,      /**< Generic error */
    W5500_SOCK_BUSY = -2,       /**< Socket is busy */
    W5500_SOCK_TIMEOUT = -3,    /**< Operation timed out */
    W5500_SOCK_BUFFER_ERROR = -4 /**< Buffer error */
} w5500_sock_error_t;

/*============================================================================*/
/*                         SOCKET MANAGEMENT                                  */
/*============================================================================*/

/**
 * @brief Creates and opens a new socket
 * 
 * @param sock_num  Socket number (0-7 for W5500)
 * @param type      Socket type (TCP/UDP)
 * @param port      Local port number
 * @return int8_t   Socket number on success, negative error code on failure
 */
int8_t w5500_socket_open(uint8_t sock_num, w5500_sock_type_t type, uint16_t port);

/**
 * @brief Close a socket
 * 
 * @param sock_num  Socket number to close
 * @return int8_t   0 on success, negative error code on failure
 */
int8_t w5500_socket_close(uint8_t sock_num);

/**
 * @brief Set socket option
 * 
 * @param sock_num     Socket number
 * @param option_type  Option type code
 * @param option_value Pointer to option value
 * @return int8_t      0 on success, negative error code on failure
 */
int8_t w5500_socket_setsockopt(uint8_t sock_num, uint8_t option_type, void *option_value);

/**
 * @brief Get socket option
 * 
 * @param sock_num     Socket number
 * @param option_type  Option type code
 * @param option_value Pointer to store option value
 * @return int8_t      0 on success, negative error code on failure
 */
int8_t w5500_socket_getsockopt(uint8_t sock_num, uint8_t option_type, void *option_value);

/*============================================================================*/
/*                         TCP SOCKET OPERATIONS                              */
/*============================================================================*/

/**
 * @brief Connect a TCP socket to a remote host
 * 
 * @param sock_num    Socket number
 * @param dest_ip     Destination IP address (4 bytes)
 * @param dest_port   Destination port number
 * @return int8_t     0 on success, negative error code on failure
 */
int8_t w5500_socket_connect(uint8_t sock_num, const uint8_t* dest_ip, uint16_t dest_port);

/**
 * @brief Listen for incoming connections on a TCP socket
 * 
 * @param sock_num    Socket number
 * @return int8_t     0 on success, negative error code on failure
 */
int8_t w5500_socket_listen(uint8_t sock_num);

/**
 * @brief Send data on a connected TCP socket
 * 
 * @param sock_num  Socket number
 * @param data      Pointer to data buffer
 * @param len       Length of data to send
 * @return int32_t  Number of bytes sent, negative error code on failure
 */
int32_t w5500_socket_send(uint8_t sock_num, const uint8_t* data, uint16_t len);

/**
 * @brief Receive data from a connected TCP socket
 * 
 * @param sock_num  Socket number
 * @param buffer    Buffer to store received data
 * @param maxlen    Maximum length of buffer
 * @return int32_t  Number of bytes received, negative error code on failure
 */
int32_t w5500_socket_recv(uint8_t sock_num, uint8_t* buffer, uint16_t maxlen);

/*============================================================================*/
/*                         UDP SOCKET OPERATIONS                              */
/*============================================================================*/

/**
 * @brief Send data to a specific destination via UDP
 * 
 * @param sock_num   Socket number
 * @param data       Pointer to data buffer
 * @param len        Length of data to send
 * @param dest_ip    Destination IP address (4 bytes)
 * @param dest_port  Destination port number
 * @return int32_t   Number of bytes sent, negative error code on failure
 */
int32_t w5500_socket_sendto(uint8_t sock_num, const uint8_t* data, uint16_t len, 
                            const uint8_t* dest_ip, uint16_t dest_port);

/**
 * @brief Receive data from a UDP socket with source information
 * 
 * @param sock_num  Socket number
 * @param buffer    Buffer to store received data
 * @param maxlen    Maximum length of buffer
 * @param src_ip    Buffer to store source IP (4 bytes)
 * @param src_port  Pointer to store source port
 * @return int32_t  Number of bytes received, negative error code on failure
 */
int32_t w5500_socket_recvfrom(uint8_t sock_num, uint8_t* buffer, uint16_t maxlen, 
                              uint8_t* src_ip, uint16_t* src_port);

/*============================================================================*/
/*                         SOCKET STATUS                                      */
/*============================================================================*/

/**
 * @brief Check if a TCP socket is connected
 * 
 * @param sock_num  Socket number
 * @return bool     true if connected, false otherwise
 */
bool w5500_socket_is_connected(uint8_t sock_num);

/**
 * @brief Get socket status
 * 
 * @param sock_num  Socket number
 * @return uint8_t  Socket status code (see socket.h for Sn_SR_* defines)
 */
uint8_t w5500_socket_get_status(uint8_t sock_num);

/**
 * @brief Get the amount of data available to be read on a socket
 * 
 * @param sock_num  Socket number
 * @return uint16_t Number of bytes available to read, 0 if none
 */
uint16_t w5500_socket_available(uint8_t sock_num);

#endif /* W5500_SOCKET_H */
