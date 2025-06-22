/**
 * @file w5500_socket.c
 * @brief W5500 Socket wrapper for STM32F103 - Pure ioLibrary wrapper
 */

#include "w5500_socket.h"
#include "eth_config.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "w5500.h"
#include <stdio.h>
#include <string.h>

static int8_t w5500_socket_get_service_socket(const char* service) {
    if (strcmp(service, "dhcp") == 0) return ETH_CONFIG_DHCP_SOCKET;
    if (strcmp(service, "tftp") == 0) return ETH_CONFIG_TFTP_SOCKET;
    if (strcmp(service, "icmp") == 0) return ETH_CONFIG_ICMP_SOCKET;
    if (strcmp(service, "mqtt") == 0) return ETH_CONFIG_MQTT_SOCKET;
    if (strcmp(service, "opcua") == 0) return ETH_CONFIG_OPCUA_SOCKET;
    if (strcmp(service, "http") == 0) return ETH_CONFIG_HTTP_SOCKET;
    if (strcmp(service, "tcp") == 0) return ETH_CONFIG_TCP_SOCKET;
    if (strcmp(service, "udp") == 0) return ETH_CONFIG_UDP_SOCKET;
    return -1;
}

bool w5500_socket_check_ready(void) {
    return (getVERSIONR() == 0x04);
}

int8_t w5500_socket_open_service(const char* service, w5500_sock_type_t type, uint16_t port) {
    int8_t socket_num = w5500_socket_get_service_socket(service);
    return (socket_num < 0) ? W5500_SOCK_ERROR : w5500_socket_open((uint8_t)socket_num, type, port);
}

int8_t w5500_socket_get_service_number(const char* service) {
    return w5500_socket_get_service_socket(service);
}

// ============================================================================
// PURE ioLibrary WRAPPER FUNCTIONS
// ============================================================================

int8_t w5500_socket_open(uint8_t sock_num, w5500_sock_type_t type, uint16_t port) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    if (!w5500_socket_check_ready()) return W5500_SOCK_ERROR;

    uint8_t protocol = (type == W5500_SOCK_TCP) ? Sn_MR_TCP : 
                      (type == W5500_SOCK_UDP) ? Sn_MR_UDP : 0;
    if (protocol == 0) return W5500_SOCK_ERROR;

    int8_t result = socket(sock_num, protocol, port, 0);
    return (result == sock_num) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

int8_t w5500_socket_close(uint8_t sock_num) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    int8_t result = close(sock_num);
    return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

int8_t w5500_socket_listen(uint8_t sock_num) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    int8_t result = listen(sock_num);
    return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

int8_t w5500_socket_connect(uint8_t sock_num, const uint8_t *dest_ip, uint16_t dest_port) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    int8_t result = connect(sock_num, (uint8_t *)dest_ip, dest_port);
    return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

int8_t w5500_socket_disconnect(uint8_t sock_num) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    int8_t result = disconnect(sock_num);
    return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

bool w5500_socket_is_established(uint8_t sock_num) {
    return (sock_num < W5500_MAX_SOCKET) && (getSn_SR(sock_num) == SOCK_ESTABLISHED);
}

int8_t w5500_socket_ctlsocket(uint8_t sock_num, uint8_t ctl_type, void *arg) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    int8_t result = ctlsocket(sock_num, (ctlsock_type)ctl_type, arg);
    return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

int8_t w5500_socket_setsockopt(uint8_t sock_num, uint8_t option_type, void *option_value) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    int8_t result = setsockopt(sock_num, (sockopt_type)option_type, option_value);
    return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

int8_t w5500_socket_getsockopt(uint8_t sock_num, uint8_t option_type, void *option_value) {
    if (sock_num >= W5500_MAX_SOCKET) return W5500_SOCK_ERROR;
    int8_t result = getsockopt(sock_num, (sockopt_type)option_type, option_value);
    return (result == SOCK_OK) ? W5500_SOCK_OK : W5500_SOCK_ERROR;
}

int32_t w5500_socket_send(uint8_t sock_num, const uint8_t *buffer, uint16_t len) {
    if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
    int32_t sent = send(sock_num, (uint8_t *)buffer, len);
    return (sent >= 0) ? sent : W5500_SOCK_ERROR;
}

int32_t w5500_socket_recv(uint8_t sock_num, uint8_t *buffer, uint16_t maxlen) {
    if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
    int32_t recvd = recv(sock_num, buffer, maxlen);
    return (recvd >= 0) ? recvd : W5500_SOCK_ERROR;
}

int32_t w5500_socket_sendto(uint8_t sock_num, const uint8_t *buffer, uint16_t len, const uint8_t *dest_ip, uint16_t dest_port) {
    if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
    int32_t sent = sendto(sock_num, (uint8_t *)buffer, len, (uint8_t *)dest_ip, dest_port);
    return (sent >= 0) ? sent : W5500_SOCK_ERROR;
}

int32_t w5500_socket_recvfrom(uint8_t sock_num, uint8_t *buffer, uint16_t maxlen, uint8_t *src_ip, uint16_t *src_port) {
    if (sock_num >= W5500_MAX_SOCKET || !buffer) return W5500_SOCK_ERROR;
    int32_t recvd = recvfrom(sock_num, buffer, maxlen, src_ip, src_port);
    return (recvd >= 0) ? recvd : W5500_SOCK_ERROR;
}

uint8_t w5500_socket_get_status(uint8_t sock_num) {
    return (sock_num < W5500_MAX_SOCKET) ? getSn_SR(sock_num) : 0xFF;
}

uint16_t w5500_socket_get_tx_buf_free_size(uint8_t sock_num) {
    return (sock_num < W5500_MAX_SOCKET) ? getSn_TX_FSR(sock_num) : 0;
}

uint16_t w5500_socket_get_rx_buf_size(uint8_t sock_num) {
    return (sock_num < W5500_MAX_SOCKET) ? getSn_RX_RSR(sock_num) : 0;
}