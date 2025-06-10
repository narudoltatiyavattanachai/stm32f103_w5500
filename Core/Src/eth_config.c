#include "eth_config.h"
#include <string.h> // Required for memcpy


/*============================================================================*/
/*                         W5500 HARDWARE PIN DEFINITIONS                     */
/*============================================================================*/

/* W5500 Ethernet Controller Pin Definitions */
//#define ETH_CONFIG_DHCP_SOCKET    1


/*============================================================================*/
/*                         W5500 HARDWARE PIN DEFINITIONS                     */
/*============================================================================*/
/**
 * @brief Global network information structure instance.
 * This is the actual definition of the extern variable declared in eth_config.h.
 */
wiz_NetInfo g_network_info;

/**
 * @brief Initializes the global network information structure with default static values.
 */
void eth_config_init_static(void) {
    // Default MAC Address
    g_network_info.mac[0] = 0x00;
    g_network_info.mac[1] = 0x08;
    g_network_info.mac[2] = 0xdc;
    g_network_info.mac[3] = 0x00;
    g_network_info.mac[4] = 0x00;
    g_network_info.mac[5] = 0x01;

    // Static IP Address
    g_network_info.ip[0] = 192;
    g_network_info.ip[1] = 168;
    g_network_info.ip[2] = 68;
    g_network_info.ip[3] = 200;

    // Subnet Mask
    g_network_info.sn[0] = 255;
    g_network_info.sn[1] = 255;
    g_network_info.sn[2] = 255;
    g_network_info.sn[3] = 0;

    // Gateway
    g_network_info.gw[0] = 192;
    g_network_info.gw[1] = 168;
    g_network_info.gw[2] = 68;
    g_network_info.gw[3] = 1;

    // DNS Server
    g_network_info.dns[0] = 8;
    g_network_info.dns[1] = 8;
    g_network_info.dns[2] = 8;
    g_network_info.dns[3] = 8;

    // DHCP mode
    g_network_info.dhcp = NETINFO_STATIC;
}

/**
 * @brief Updates the W5500 chip with provided network info.
 */
void eth_config_set_netinfo(const wiz_NetInfo* net_info) {
    memcpy(&g_network_info, net_info, sizeof(wiz_NetInfo));
    wizchip_setnetinfo(&g_network_info);
}

/**
 * @brief Retrieves current network configuration from W5500 chip.
 */
void eth_config_get_netinfo(wiz_NetInfo* net_info) {
    wizchip_getnetinfo(&g_network_info);
    memcpy(net_info, &g_network_info, sizeof(wiz_NetInfo));
}
