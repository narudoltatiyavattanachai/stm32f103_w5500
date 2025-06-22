/**
 * @file w5500_socket.h
 * @brief W5500 Ethernet socket interface for STM32G4xx
 *
 * @details This module wraps the WIZnet ioLibrary_Driver SOCKET API for use on STM32G4xx.
 *          It provides simple, high-level socket open, close, connect, send/recv,
 *          plus option management and status utilities.
 *
 * @author
 * @date 2025-06-18
 */

 #ifndef _W5500_SOCKET_H_
 #define _W5500_SOCKET_H_
 
 #include <stdint.h>
 #include <stdbool.h>
 #include <stdio.h>
 #include <string.h>
 
 #include "wizchip_conf.h"
 #include "socket.h"

/*============================================================================*/
/*                         DEBUG CONFIGURATION                               */
/*============================================================================*/
 /**
  * @brief Maximum number of sockets supported by W5500
  */
 #define W5500_MAX_SOCKET 8
 
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
     W5500_SOCK_OK = 0,           /**< Operation successful */
     W5500_SOCK_ERROR = -1,       /**< Generic error */
     W5500_SOCK_BUSY = -2,        /**< Socket busy */
     W5500_SOCK_TIMEOUT = -3,     /**< Timeout occurred */
     W5500_SOCK_BUFFER_ERROR = -4 /**< Buffer error */
 } w5500_sock_error_t;
 
 /*============================================================================*/
 /* COMPATIBILITY AND INITIALIZATION */
 /*============================================================================*/

 /**
  * @brief Check if W5500 is ready for socket operations
  * @return true if W5500 SPI is initialized and working, false otherwise
  */
 bool w5500_socket_check_ready(void);

 /**
  * @brief Open socket for specific service using centralized config
  * @param service Service name ("dhcp", "tftp", "icmp", "mqtt", "opcua", "http", "tcp", "udp")
  * @param type Socket type (TCP or UDP)
  * @param port Port number
  * @return W5500_SOCK_OK on success, error code otherwise
  */
 int8_t w5500_socket_open_service(const char* service, w5500_sock_type_t type, uint16_t port);

 /**
  * @brief Get socket number for a specific service
  * @param service Service name
  * @return Socket number or -1 if invalid service
  */
 int8_t w5500_socket_get_service_number(const char* service);

 /**
  * @brief Send TCP message using centralized socket configuration
  * @param service Service name (e.g., "tcp")
  * @param dest_ip Destination IP address
  * @param dest_port Destination port
  * @param message Message to send
  * @param len Length of message
  * @return Number of bytes sent or error code
  */
 int32_t w5500_socket_send_tcp_message(const char* service, const uint8_t* dest_ip, uint16_t dest_port, const uint8_t* message, uint16_t len);

 /**
  * @brief Send UDP hello world message using centralized configuration from eth_config.h
  * @return Number of bytes sent or error code
  */
 int32_t w5500_socket_send_udp_hello_world(void);

 /*============================================================================*/
 /* SOCKET MANAGEMENT */
 /*============================================================================*/
 
 /**
  * @brief Open and configure a socket
  */
 int8_t w5500_socket_open(uint8_t sock_num, w5500_sock_type_t type, uint16_t port);
 
 /**
  * @brief Close a socket
  */
 int8_t w5500_socket_close(uint8_t sock_num);
 
 /**
  * @brief Start listening for incoming TCP connections
  */
 int8_t w5500_socket_listen(uint8_t sock_num);
 
 /**
  * @brief Connect a TCP socket to a remote host
  */
 int8_t w5500_socket_connect(uint8_t sock_num, const uint8_t* dest_ip, uint16_t dest_port);
 
 /**
  * @brief Gracefully disconnect a TCP socket
  */
 int8_t w5500_socket_disconnect(uint8_t sock_num);
 
 /*============================================================================*/
 /* OPTIONS & CONTROL */
 /*============================================================================*/
 
 /**
  * @brief Control socket I/O mode and interrupts (ctlsocket equivalent)
  */
 int8_t w5500_socket_ctlsocket(uint8_t sock_num, uint8_t ctl_type, void *arg);
 
 /**
  * @brief Set a socket option (setsockopt)
  */
 int8_t w5500_socket_setsockopt(uint8_t sock_num, uint8_t option_type, void *option_value);
 
 /**
  * @brief Get a socket option (getsockopt)
  */
 int8_t w5500_socket_getsockopt(uint8_t sock_num, uint8_t option_type, void *option_value);
 
 /*============================================================================*/
 /* DATA TRANSFER */
 /*============================================================================*/
 
 /**
  * @brief Send data (TCP/UDP)
  */
 int32_t w5500_socket_send(uint8_t sock_num, const uint8_t* buffer, uint16_t len);
 
 /**
  * @brief Receive data (TCP/UDP)
  */
 int32_t w5500_socket_recv(uint8_t sock_num, uint8_t* buffer, uint16_t maxlen);
 
 /**
  * @brief Send UDP data to a specified IP and port
  */
 int32_t w5500_socket_sendto(uint8_t sock_num, const uint8_t* buffer, uint16_t len,
                             const uint8_t* dest_ip, uint16_t dest_port);
 
 /**
  * @brief Receive UDP data, get source IP and port
  */
 int32_t w5500_socket_recvfrom(uint8_t sock_num, uint8_t* buffer, uint16_t maxlen,
                               uint8_t* src_ip, uint16_t* src_port);
 
 /*============================================================================*/
 /* STATUS HELPERS */
 /*============================================================================*/
 
 /**
  * @brief Check if TCP socket is in ESTABLISHED state
  */
 bool w5500_socket_is_established(uint8_t sock_num);
 
 /**
  * @brief Get current socket status register (Sn_SR)
  */
 uint8_t w5500_socket_get_status(uint8_t sock_num);
 
 /**
  * @brief Get amount of free TX buffer space
  */
 uint16_t w5500_socket_get_tx_buf_free_size(uint8_t sock_num);
 
 /**
  * @brief Get amount of received RX buffer data
  */
 uint16_t w5500_socket_get_rx_buf_size(uint8_t sock_num);
 
 #endif // _W5500_SOCKET_H_
 
