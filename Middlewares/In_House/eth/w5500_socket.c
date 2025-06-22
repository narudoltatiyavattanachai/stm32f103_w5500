/**
 * @file w5500_socket.c
 * @brief W5500 Ethernet socket wrapper for STM32G4xx
 *
 * @details This wraps WIZnet ioLibrary_Driver SOCKET API to provide
 *          a safe STM32-compatible socket layer.
 *
 * @date 2025-06-18
 * @version 1.1
 */

 #include "w5500_socket.h"
 #include "w5500_spi.h"
 #include "eth_config.h"
 #include "wizchip_conf.h"
 #include "socket.h"
 #include "w5500.h"
 #include <stdio.h>
 #include <string.h>

 //=============================================================================
 // COMPATIBILITY LAYER
 //=============================================================================

 /**
  * @brief Get socket number for specific service from centralized config
  * @param service Service type identifier
  * @return Socket number or -1 if invalid service
  */
 static int8_t w5500_socket_get_service_socket(const char* service)
 {
     if (strcmp(service, "dhcp") == 0) return ETH_CONFIG_DHCP_SOCKET;
     if (strcmp(service, "tftp") == 0) return ETH_CONFIG_TFTP_SOCKET;
     if (strcmp(service, "icmp") == 0) return ETH_CONFIG_ICMP_SOCKET;
     if (strcmp(service, "mqtt") == 0) return ETH_CONFIG_MQTT_SOCKET;
     if (strcmp(service, "opcua") == 0) return ETH_CONFIG_OPCUA_SOCKET;
     if (strcmp(service, "http") == 0) return ETH_CONFIG_HTTP_SOCKET;
     if (strcmp(service, "tcp") == 0) return ETH_CONFIG_TCP_SOCKET;
     if (strcmp(service, "udp") == 0) return ETH_CONFIG_UDP_SOCKET;
     return -1; // Invalid service
 }
 
 /**
  * @brief Check if W5500 is properly initialized
  * @return true if initialized, false otherwise
  */
 static bool w5500_socket_is_initialized(void)
 {
     // Check if W5500 SPI interface is working by reading version register
     uint8_t version = getVERSIONR();
     return (version == 0x04); // W5500 version should be 0x04
 }

 /**
  * @brief Public function to check if W5500 is ready for socket operations
  * @return true if W5500 SPI is initialized and working, false otherwise
  */
 bool w5500_socket_check_ready(void)
 {
     return w5500_socket_is_initialized();
 }

 /**
  * @brief Open socket for specific service using centralized config
  */
 int8_t w5500_socket_open_service(const char* service, w5500_sock_type_t type, uint16_t port)
 {
     int8_t socket_num = w5500_socket_get_service_socket(service);
     if (socket_num < 0) {
         printf("w5500_socket_open_service: Invalid service '%s'\r\n", service);
         return W5500_SOCK_ERROR;
     }
     
     printf("w5500_socket_open_service: Opening socket %d for service '%s'\r\n", socket_num, service);
     return w5500_socket_open((uint8_t)socket_num, type, port);
 }

 /**
  * @brief Get socket number for a specific service
  */
 int8_t w5500_socket_get_service_number(const char* service)
 {
     return w5500_socket_get_service_socket(service);
 }

 /**
  * @brief Send TCP message using centralized socket configuration
  */
 int32_t w5500_socket_send_tcp_message(const char* service, const uint8_t* dest_ip, uint16_t dest_port, const uint8_t* message, uint16_t len)
 {
     int8_t socket_num = w5500_socket_get_service_socket(service);
     if (socket_num < 0) {
         printf("w5500_socket_send_tcp_message: Invalid service '%s'\r\n", service);
         return W5500_SOCK_ERROR;
     }

     // Check if W5500 is initialized
     if (!w5500_socket_is_initialized()) {
         printf("w5500_socket_send_tcp_message: W5500 not initialized\r\n");
         return W5500_SOCK_ERROR;
     }

     printf("w5500_socket_send_tcp_message: Using socket %d for service '%s'\r\n", socket_num, service);
     
     // Open socket as TCP
     int8_t result = w5500_socket_open((uint8_t)socket_num, W5500_SOCK_TCP, 0);
     if (result != W5500_SOCK_OK) {
         printf("w5500_socket_send_tcp_message: Failed to open socket\r\n");
         return result;
     }

     // Connect to destination
     result = w5500_socket_connect((uint8_t)socket_num, dest_ip, dest_port);
     if (result != W5500_SOCK_OK) {
         printf("w5500_socket_send_tcp_message: Failed to connect\r\n");
         w5500_socket_close((uint8_t)socket_num);
         return result;
     }

     // Wait for connection establishment
     for (int i = 0; i < 100; i++) {
         if (w5500_socket_is_established((uint8_t)socket_num)) {
             break;
         }
         // Small delay - use HAL_Delay to avoid FreeRTOS dependency during init
         for (volatile int j = 0; j < 10000; j++);
     }

     if (!w5500_socket_is_established((uint8_t)socket_num)) {
         printf("w5500_socket_send_tcp_message: Connection timeout\r\n");
         w5500_socket_close((uint8_t)socket_num);
         return W5500_SOCK_TIMEOUT;
     }

     // Send message
     int32_t sent = w5500_socket_send((uint8_t)socket_num, message, len);
     
     // Cleanup
     w5500_socket_disconnect((uint8_t)socket_num);
     w5500_socket_close((uint8_t)socket_num);

     return sent;
 }

 /**
  * @brief Send UDP hello world message using centralized configuration from eth_config.h
  */
 int32_t w5500_socket_send_udp_hello_world(void)
 {
     // Use centralized configuration from eth_config.h
     uint8_t target_ip[] = ETH_CONFIG_UDP_TARGET_IP;
     uint16_t target_port = ETH_CONFIG_UDP_TARGET_PORT;
     const char* message = ETH_CONFIG_UDP_MESSAGE;
     uint8_t socket_num = ETH_CONFIG_UDP_SOCKET;

     // Check if W5500 is initialized
     if (!w5500_socket_is_initialized()) {
         printf("w5500_socket_send_udp_hello_world: W5500 not initialized\r\n");
         return W5500_SOCK_ERROR;
     }

     printf("w5500_socket_send_udp_hello_world: Sending '%s' to %d.%d.%d.%d:%d via socket %d\r\n", 
            message, target_ip[0], target_ip[1], target_ip[2], target_ip[3], target_port, socket_num);

     // Open UDP socket
     int8_t result = w5500_socket_open(socket_num, W5500_SOCK_UDP, 0);
     if (result != W5500_SOCK_OK) {
         printf("w5500_socket_send_udp_hello_world: Failed to open UDP socket\r\n");
         return result;
     }

     // Send UDP message
     int32_t sent = w5500_socket_sendto(socket_num, (const uint8_t*)message, strlen(message), target_ip, target_port);
     
     if (sent > 0) {
         printf("w5500_socket_send_udp_hello_world: Successfully sent %ld bytes\r\n", sent);
     } else {
         printf("w5500_socket_send_udp_hello_world: Failed to send UDP message: %ld\r\n", sent);
     }

     // Close socket
     w5500_socket_close(socket_num);

     return sent;
 }

 //=============================================================================
 // SOCKET MANAGEMENT
 //=============================================================================
 
 int8_t w5500_socket_open(uint8_t sock_num, w5500_sock_type_t type, uint16_t port)
 {
     if (sock_num >= W5500_MAX_SOCKET) {
         printf("w5500_socket_open: Invalid socket %d\r\n", sock_num);
         return W5500_SOCK_ERROR;
     }

     // Check if W5500 SPI is initialized
     if (!w5500_socket_is_initialized()) {
         printf("w5500_socket_open: W5500 not initialized\r\n");
         return W5500_SOCK_ERROR;
     }
 
     uint8_t protocol = 0;
     switch (type) {
         case W5500_SOCK_TCP: protocol = Sn_MR_TCP; break;
         case W5500_SOCK_UDP: protocol = Sn_MR_UDP; break;
         default:
             printf("w5500_socket_open: Invalid type %d\r\n", type);
             return W5500_SOCK_ERROR;
     }
 
     printf("w5500_socket_open: socket=%d type=%d port=%d\r\n", sock_num, type, port);
 
     int8_t result = socket(sock_num, protocol, port, 0);
     return (result == sock_num) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 int8_t w5500_socket_close(uint8_t sock_num)
 {
     if (sock_num >= W5500_MAX_SOCKET) {
         printf("w5500_socket_close: Invalid socket %d\r\n", sock_num);
         return W5500_SOCK_ERROR;
     }
 
     int8_t result = close(sock_num);
     return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 int8_t w5500_socket_listen(uint8_t sock_num)
 {
     if (sock_num >= W5500_MAX_SOCKET) {
         printf("w5500_socket_listen: Invalid socket %d\r\n", sock_num);
         return W5500_SOCK_ERROR;
     }
 
     int8_t result = listen(sock_num);
     return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 int8_t w5500_socket_connect(uint8_t sock_num, const uint8_t *dest_ip, uint16_t dest_port)
 {
     if (sock_num >= W5500_MAX_SOCKET) {
         printf("w5500_socket_connect: Invalid socket %d\r\n", sock_num);
         return W5500_SOCK_ERROR;
     }
 
     int8_t result = connect(sock_num, (uint8_t *)dest_ip, dest_port);
     return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 int8_t w5500_socket_disconnect(uint8_t sock_num)
 {
     if (sock_num >= W5500_MAX_SOCKET) {
         printf("w5500_socket_disconnect: Invalid socket %d\r\n", sock_num);
         return W5500_SOCK_ERROR;
     }
 
     int8_t result = disconnect(sock_num);
     return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 bool w5500_socket_is_established(uint8_t sock_num)
 {
     if (sock_num >= W5500_MAX_SOCKET) return false;
     return (getSn_SR(sock_num) == SOCK_ESTABLISHED);
 }
 
 //=============================================================================
 // CONTROL & OPTIONS
 //=============================================================================
 
 int8_t w5500_socket_ctlsocket(uint8_t sock_num, uint8_t ctl_type, void *arg)
 {
     if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
 
     int8_t result = ctlsocket(sock_num, (ctlsock_type)ctl_type, arg);
     return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 int8_t w5500_socket_setsockopt(uint8_t sock_num, uint8_t option_type, void *option_value)
 {
     if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
 
     int8_t result = setsockopt(sock_num, (sockopt_type)option_type, option_value);
     return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 int8_t w5500_socket_getsockopt(uint8_t sock_num, uint8_t option_type, void *option_value)
 {
     if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
 
     int8_t result = getsockopt(sock_num, (sockopt_type)option_type, option_value);
     return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
 }
 
 //=============================================================================
 // DATA TRANSFER
 //=============================================================================
 
 int32_t w5500_socket_send(uint8_t sock_num, const uint8_t *buffer, uint16_t len)
 {
     if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
 
     int32_t sent = send(sock_num, (uint8_t *)buffer, len);
     return (sent >= 0) ? sent : W5500_SOCK_ERROR;
 }
 
 int32_t w5500_socket_recv(uint8_t sock_num, uint8_t *buffer, uint16_t maxlen)
 {
     if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
 
     int32_t recvd = recv(sock_num, buffer, maxlen);
     return (recvd >= 0) ? recvd : W5500_SOCK_ERROR;
 }
 
 int32_t w5500_socket_sendto(uint8_t sock_num, const uint8_t *buffer, uint16_t len,
                             const uint8_t *dest_ip, uint16_t dest_port)
 {
     if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
 
     int32_t sent = sendto(sock_num, (uint8_t *)buffer, len, (uint8_t *)dest_ip, dest_port);
     return (sent >= 0) ? sent : W5500_SOCK_ERROR;
 }
 
 int32_t w5500_socket_recvfrom(uint8_t sock_num, uint8_t *buffer, uint16_t maxlen,
                               uint8_t *src_ip, uint16_t *src_port)
 {
     if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
 
     int32_t recvd = recvfrom(sock_num, buffer, maxlen, src_ip, src_port);
     return (recvd >= 0) ? recvd : W5500_SOCK_ERROR;
 }
 
 //=============================================================================
 // STATUS HELPERS
 //=============================================================================
 
 uint8_t w5500_socket_get_status(uint8_t sock_num)
 {
     if (sock_num >= W5500_MAX_SOCKET) return 0xFF;
     return getSn_SR(sock_num);
 }
 
 uint16_t w5500_socket_get_tx_buf_free_size(uint8_t sock_num)
 {
     if (sock_num >= W5500_MAX_SOCKET) return 0;
     return getSn_TX_FSR(sock_num);
 }
 
 uint16_t w5500_socket_get_rx_buf_size(uint8_t sock_num)
 {
     if (sock_num >= W5500_MAX_SOCKET) return 0;
     return getSn_RX_RSR(sock_num);
 }
 
