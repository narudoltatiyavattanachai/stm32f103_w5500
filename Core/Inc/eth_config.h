#ifndef _ETH_CONFIG_H_
#define _ETH_CONFIG_H_

#include "wizchip_conf.h"

// Global network configuration instance
extern wiz_NetInfo g_network_info;

// Initialization
void eth_config_init_static(void);

// Set and get network configuration
void eth_config_set_netinfo(const wiz_NetInfo* net_info);
void eth_config_get_netinfo(wiz_NetInfo* net_info);

#endif // ETH_CONFIG_H
