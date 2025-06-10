#ifndef _ETH_CONFIG_H_
#define _ETH_CONFIG_H_

#include "../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/wizchip_conf.h"

// Network Configuration Macros
#define ETH_CONFIG_MAC_STR     "00:08:DC:12:34:56"  // Default MAC address as string
#define ETH_CONFIG_IP_STR      "192.168.68.200"     // Default static IP as string
#define ETH_CONFIG_SUBNET_STR  "255.255.255.0"     // Default subnet mask as string
#define ETH_CONFIG_GATEWAY_STR "192.168.68.1"      // Default gateway as string
#define ETH_CONFIG_DNS_STR     "8.8.8.8"           // Default DNS server as string

// Network configuration in byte arrays (initialized in eth_config.c)
extern const uint8_t ETH_CONFIG_MAC[6];
extern const uint8_t ETH_CONFIG_IP[4];
extern const uint8_t ETH_CONFIG_SUBNET[4];
extern const uint8_t ETH_CONFIG_GATEWAY[4];
extern const uint8_t ETH_CONFIG_DNS[4];

// Global network configuration instance
extern wiz_NetInfo g_network_info;

// Initialization
void eth_config_init_static(void);

// Set and get network configuration
void eth_config_set_netinfo(const wiz_NetInfo* net_info);
void eth_config_get_netinfo(wiz_NetInfo* net_info);

#endif // ETH_CONFIG_H
