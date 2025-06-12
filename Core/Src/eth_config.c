#include "eth_config.h"
#include <string.h>  // For memcpy
#include <stdio.h>   // For printf

// Global network configuration structure
wiz_NetInfo g_network_info;

/**
 * @brief Initializes g_network_info with static IP settings from eth_config.h
 */
void eth_config_init_static(void) {
    memcpy(g_network_info.mac,     ETH_CONFIG_MAC,     sizeof(g_network_info.mac));
    memcpy(g_network_info.ip,      ETH_CONFIG_IP,      sizeof(g_network_info.ip));
    memcpy(g_network_info.sn,      ETH_CONFIG_SUBNET,  sizeof(g_network_info.sn));
    memcpy(g_network_info.gw,      ETH_CONFIG_GATEWAY, sizeof(g_network_info.gw));
    memcpy(g_network_info.dns,     ETH_CONFIG_DNS,     sizeof(g_network_info.dns));

    g_network_info.dhcp = NETINFO_STATIC;

    printf("Initialized g_network_info with static values.\r\n");
}

/**
 * @brief Apply the provided network settings to the W5500 chip
 */
void eth_config_set_netinfo(const wiz_NetInfo* net_info) {

    memcpy(&g_network_info, net_info, sizeof(wiz_NetInfo));

    // Apply all at once
    wizchip_setnetinfo(&g_network_info);

    HAL_Delay(10);  // Let it settle

    wiz_NetInfo confirm;
    wizchip_getnetinfo(&confirm);
    printf("Confirmed from chip: IP %d.%d.%d.%d, MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           confirm.ip[0], confirm.ip[1], confirm.ip[2], confirm.ip[3],
           confirm.mac[0], confirm.mac[1], confirm.mac[2],
           confirm.mac[3], confirm.mac[4], confirm.mac[5]);
}




/**
 * @brief Read the current W5500 configuration from the chip
 */
void eth_config_get_netinfo(wiz_NetInfo* net_info) {
    wizchip_getnetinfo(net_info);
    printf("Current W5500 Net Info: IP %d.%d.%d.%d, MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           net_info->ip[0], net_info->ip[1], net_info->ip[2], net_info->ip[3],
           net_info->mac[0], net_info->mac[1], net_info->mac[2],
           net_info->mac[3], net_info->mac[4], net_info->mac[5]);
}
