/**
 * @file hello_world.c
 * @brief UDP Hello World utility implementation
 */

#include "hello_world.h"
#include "w5500_socket.h"
#include "eth_config.h"
#include <string.h>
#include <stdio.h>

int32_t hello_world_send_udp(void) {
    uint8_t target_ip[] = ETH_CONFIG_UDP_TARGET_IP;
    uint16_t target_port = ETH_CONFIG_UDP_TARGET_PORT;
    const char* message = ETH_CONFIG_UDP_MESSAGE;
    uint8_t socket_num = ETH_CONFIG_UDP_SOCKET;

    if (!w5500_socket_check_ready()) return -1;
    if (w5500_socket_open(socket_num, W5500_SOCK_UDP, 0) != W5500_SOCK_OK) return -2;
    
    int32_t sent = w5500_socket_sendto(socket_num, (const uint8_t*)message, strlen(message), target_ip, target_port);
    w5500_socket_close(socket_num);
    return sent;
}

int32_t hello_world_send_tcp(const uint8_t* dest_ip, uint16_t dest_port) {
    const char* message = ETH_CONFIG_UDP_MESSAGE; // Reuse same message
    uint8_t socket_num = ETH_CONFIG_TCP_SOCKET;

    if (!w5500_socket_check_ready() || !dest_ip) return -1;
    if (w5500_socket_open(socket_num, W5500_SOCK_TCP, 0) != W5500_SOCK_OK) return -2;
    if (w5500_socket_connect(socket_num, dest_ip, dest_port) != W5500_SOCK_OK) {
        w5500_socket_close(socket_num);
        return -3;
    }
    
    // Wait for connection
    for (int i = 0; i < 100 && !w5500_socket_is_established(socket_num); i++) {
        for (volatile int j = 0; j < 10000; j++);
    }
    
    if (!w5500_socket_is_established(socket_num)) {
        w5500_socket_close(socket_num);
        return -4;
    }
    
    int32_t sent = w5500_socket_send(socket_num, (const uint8_t*)message, strlen(message));
    w5500_socket_disconnect(socket_num);
    w5500_socket_close(socket_num);
    return sent;
}