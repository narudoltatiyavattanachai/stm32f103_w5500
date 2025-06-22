#ifndef _ETH_CONFIG_H_
#define _ETH_CONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <cmsis_os2.h>
#include "stm32f1xx_hal.h"

#include "wizchip_conf.h"



// === Static const array definitions (usable in C code) ===
static const uint8_t ETH_CONFIG_MAC[6]     = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
static const uint8_t ETH_CONFIG_IP[4]      = {192, 168, 100, 151};
static const uint8_t ETH_CONFIG_SUBNET[4]  = {255, 255, 255, 0};
static const uint8_t ETH_CONFIG_GATEWAY[4] = {192, 168, 100, 1};
static const uint8_t ETH_CONFIG_DNS[4]     = {8, 8, 8, 8};

// === Global configuration structure ===
extern wiz_NetInfo g_network_info;

// === Configuration functions ===
void eth_config_init_static(void);
void eth_config_set_netinfo(const wiz_NetInfo* net_info);
void eth_config_get_netinfo(wiz_NetInfo* net_info);

#endif // _ETH_CONFIG_H_
