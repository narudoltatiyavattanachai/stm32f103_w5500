#ifndef _ETH_CONFIG_H_
#define _ETH_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/wizchip_conf.h"

// === String literals (optional use) ===
//#define ETH_CONFIG_MAC_STR     "00:08:DC:12:34:56"
//#define ETH_CONFIG_IP_STR      "192.168.68.200"
//#define ETH_CONFIG_SUBNET_STR  "255.255.255.0"
//#define ETH_CONFIG_GATEWAY_STR "192.168.68.1"
//#define ETH_CONFIG_DNS_STR     "8.8.8.8"

// === Static const array definitions (usable in C code) ===
static const uint8_t ETH_CONFIG_MAC[6]     = {0x00, 0x08, 0xDC, 0x00, 0x00, 0x01};
static const uint8_t ETH_CONFIG_IP[4]      = {192, 168, 68, 200};
static const uint8_t ETH_CONFIG_SUBNET[4]  = {255, 255, 255, 0};
static const uint8_t ETH_CONFIG_GATEWAY[4] = {192, 168, 68, 1};
static const uint8_t ETH_CONFIG_DNS[4]     = {8, 8, 8, 8};

// === Global configuration structure ===
extern wiz_NetInfo g_network_info;

// === Configuration functions ===
void eth_config_init_static(void);
void eth_config_set_netinfo(const wiz_NetInfo* net_info);
void eth_config_get_netinfo(wiz_NetInfo* net_info);

#endif // _ETH_CONFIG_H_
