/**
 * @file w5500_socket.c
 * @brief Socket API wrapper implementation for W5500 Ethernet controller
 * 
 * @details This file implements wrapper functions around the third-party socket
 *          API to isolate application code from direct dependencies on
 *          third-party libraries. All socket operations should use these
 *          wrapper functions instead of calling socket.h functions directly.
 */

 #include "w5500_socket.h"
 #include "socket.h"
 #include <string.h>
 #include "stm32f1xx_hal.h"
 
 /* Debug print macro - comment out to disable debug messages */
 #ifdef W5500_DEBUG
 #define DEBUG_PRINT(...)  printf(__VA_ARGS__)
 #else
 #define DEBUG_PRINT(...)
 #endif
 
 /* ==========================================================================
  * SOCKET API WRAPPER FUNCTIONS
  * These wrappers isolate application code from the third-party socket library
  * ==========================================================================*/
 
 /**
  * @brief Create a socket with the specified parameters
  */
 int8_t w5500_socket(uint8_t socket_num, uint8_t protocol, uint16_t port, uint8_t flag)
 {
     DEBUG_PRINT("w5500_socket: Creating socket %d with protocol %d port %d flag %d\r\n", 
                 socket_num, protocol, port, flag);
     return socket(socket_num, protocol, port, flag);
 }
 
 /**
  * @brief Close a socket
  */
 int8_t w5500_close(uint8_t socket_num)
 {
     DEBUG_PRINT("w5500_close: Closing socket %d\r\n", socket_num);
     return close(socket_num);
 }
 
 /**
  * @brief Listen for incoming connections on a socket
  */
 int8_t w5500_listen(uint8_t socket_num)
 {
     DEBUG_PRINT("w5500_listen: Listening on socket %d\r\n", socket_num);
     return listen(socket_num);
 }
 
 /**
  * @brief Connect to a remote host
  */
 int8_t w5500_connect(uint8_t socket_num, const uint8_t *remote_ip, uint16_t remote_port)
 {
     DEBUG_PRINT("w5500_connect: Connecting socket %d to %d.%d.%d.%d:%d\r\n", 
                 socket_num, remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
     /* Cast away const qualifier as third-party API doesn't use const, but remote_ip won't be modified */
     return connect(socket_num, (uint8_t*)remote_ip, remote_port);
 }
 
 /**
  * @brief Disconnect a TCP connection
  */
 int8_t w5500_disconnect(uint8_t socket_num)
 {
     DEBUG_PRINT("w5500_disconnect: Disconnecting socket %d\r\n", socket_num);
     return disconnect(socket_num);
 }
 
 /**
  * @brief Send data through a connected TCP socket
  */
 int32_t w5500_send(uint8_t socket_num, const uint8_t *data, uint16_t data_len)
 {
     DEBUG_PRINT("w5500_send: Sending %d bytes on socket %d\r\n", data_len, socket_num);
     /* Cast away const qualifier as third-party API doesn't use const, but data won't be modified */
     return send(socket_num, (uint8_t*)data, data_len);
 }
 
 /**
  * @brief Receive data from a connected TCP socket
  */
 int32_t w5500_recv(uint8_t socket_num, uint8_t *data, uint16_t data_len)
 {
     int32_t ret = recv(socket_num, data, data_len);
     if (ret > 0) {
         DEBUG_PRINT("w5500_recv: Received %d bytes on socket %d\r\n", ret, socket_num);
     }
     return ret;
 }
 
 /**
  * @brief Send data through a UDP socket to a specific destination
  */
 int32_t w5500_sendto(uint8_t socket_num, const uint8_t *data, uint16_t data_len, 
                     const uint8_t *remote_ip, uint16_t remote_port)
 {
     DEBUG_PRINT("w5500_sendto: Sending %d bytes on socket %d to %d.%d.%d.%d:%d\r\n", 
                 data_len, socket_num, remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3], remote_port);
     /* Cast away const qualifiers as third-party API doesn't use const, but these parameters won't be modified */
     return sendto(socket_num, (uint8_t*)data, data_len, (uint8_t*)remote_ip, remote_port);
 }
 
 /**
  * @brief Receive data from a UDP socket
  */
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
 
 /**
  * @brief Set socket option
  */
 int8_t w5500_setsockopt(uint8_t socket_num, uint8_t option_type, void *option_value)
 {
     DEBUG_PRINT("w5500_setsockopt: Setting option %d on socket %d\r\n", option_type, socket_num);
     return setsockopt(socket_num, option_type, option_value);
 }
 
 /**
  * @brief Get socket option
  */
 int8_t w5500_getsockopt(uint8_t socket_num, uint8_t option_type, void *option_value)
 {
     int8_t ret = getsockopt(socket_num, option_type, option_value);
     DEBUG_PRINT("w5500_getsockopt: Getting option %d from socket %d, result %d\r\n", 
                 option_type, socket_num, ret);
     return ret;
 }
 
 /**
  * @brief Get socket status
  */
 uint8_t w5500_socket_status(uint8_t socket_num)
 {
     uint8_t status = getSn_SR(socket_num);
     DEBUG_PRINT("w5500_socket_status: Socket %d status = 0x%02X\r\n", socket_num, status);
     return status;
 }
 