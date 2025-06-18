/**
 * @file    w5500_icmp.c
 * @brief   ICMP Ping Task for W5500 Ethernet Controller
 * @author  Narudol T.
 * @date    2025-06-10
 */

 #include "w5500_icmp.h"
 #include "w5500_socket.h"
 #include "eth_config.h"
 #include <string.h>
 #include <stdio.h>
 
 #ifndef ETH_CONFIG_ICMP_SOCKET
 #define ETH_CONFIG_ICMP_SOCKET 1
 #endif
 
 #ifndef ETH_CONFIG_ICMP_BUF_LEN
 #define ETH_CONFIG_ICMP_BUF_LEN 64
 #endif
 
 static const uint8_t PC_PING_IP[4] = {192, 168, 68, 100};
 static const char *RESPONSE = "pong";
 static const char *KEYWORD  = "ping";
 
 static uint8_t icmp_socket = ETH_CONFIG_ICMP_SOCKET;
 static uint8_t icmp_buffer[ETH_CONFIG_ICMP_BUF_LEN];
 
 void w5500_icmp_init(void) {

     // Open UDP socket on arbitrary port (0 = any)
     w5500_socket_close(icmp_socket);

     if (w5500_socket_open(icmp_socket, W5500_SOCK_UDP, 0) != icmp_socket) 
     {

     }
 }
 
 void w5500_icmp_task100ms(void) 
 {
     int32_t len;
     uint8_t src_ip[4];
     uint16_t src_port;
 
     len = w5500_socket_recvfrom(icmp_socket, icmp_buffer, sizeof(icmp_buffer), src_ip, &src_port);

     if (len > 0 && memcmp(src_ip, PC_PING_IP, 4) == 0) 
     {
         if (len >= strlen(KEYWORD) && memcmp(icmp_buffer, KEYWORD, strlen(KEYWORD)) == 0) {
             w5500_socket_sendto(icmp_socket, (const uint8_t *)RESPONSE, strlen(RESPONSE), src_ip, src_port);
         }
     }
 }