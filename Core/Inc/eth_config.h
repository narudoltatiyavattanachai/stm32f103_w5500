#ifndef _ETH_CONFIG_H_
#define _ETH_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/wizchip_conf.h"

// === WIZnet ioLibrary Configuration ===
#define _WIZCHIP_                W5500
#define _WIZCHIP_IO_MODE_        _WIZCHIP_IO_MODE_SPI_VDM_

// === Network Buffer Configuration ===
#define ETH_CONFIG_BUFFER_SIZE_KB   2       // Buffer size per socket in KB
#define ETH_CONFIG_TOTAL_BUFFERS    8       // Total number of socket buffers

// === Static const array definitions (usable in C code) ===
static const uint8_t ETH_CONFIG_MAC[6]     = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
static const uint8_t ETH_CONFIG_IP[4]      = {192, 168, 100, 151};
static const uint8_t ETH_CONFIG_SUBNET[4]  = {255, 255, 255, 0};
static const uint8_t ETH_CONFIG_GATEWAY[4] = {192, 168, 100, 1};
static const uint8_t ETH_CONFIG_DNS[4]     = {8, 8, 8, 8};

// === Socket Configuration ===
#define ETH_CONFIG_DHCP_SOCKET      0       // Socket number for DHCP
#define ETH_CONFIG_TFTP_SOCKET      1       // Socket number for TFTP
#define ETH_CONFIG_ICMP_SOCKET      2       // Socket number for ICMP
#define ETH_CONFIG_MQTT_SOCKET      3       // Socket number for MQTT
#define ETH_CONFIG_OPCUA_SOCKET     4       // Socket number for OPC UA
#define ETH_CONFIG_HTTP_SOCKET      5       // Socket number for HTTP
#define ETH_CONFIG_TCP_SOCKET       6       // Socket number for general TCP
#define ETH_CONFIG_UDP_SOCKET       7       // Socket number for general UDP

// === Socket Buffer Configuration ===
#define ETH_CONFIG_SOCKET_BUFFER_SIZE 2048  // Default socket buffer size

// === UDP Hello World Configuration ===
#define ETH_CONFIG_UDP_TARGET_IP        {192, 168, 100, 131}  // Target IP for UDP hello world
#define ETH_CONFIG_UDP_TARGET_PORT      8000                   // Target port for UDP hello world
#define ETH_CONFIG_UDP_MESSAGE          "hello world"          // UDP message to send

// === Global configuration structure ===
extern wiz_NetInfo g_network_info;

// === Configuration functions ===
void eth_config_init_static(void);
void eth_config_set_netinfo(const wiz_NetInfo* net_info);
void eth_config_get_netinfo(wiz_NetInfo* net_info);

#endif // _ETH_CONFIG_H_
