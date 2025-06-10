/*============================================================================*/
/*  @file    w5500_dhcp.c                                                     */
/*  @brief   DHCP client implementation for W5500 Ethernet controller         */
/*----------------------------------------------------------------------------*/
/*  Centralized configuration using eth_config.h                              */
/*============================================================================*/

#include "w5500_dhcp.h"
#include "w5500_socket.h"
#include "../../../Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/socket.h"
#include "../../../Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/wizchip_conf.h"
#include "../../../Third_Party/ioLibrary_Driver_v3.2.0/Internet/DHCP/dhcp.h"
#include "eth_config.h"
#include <string.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"

/*============================================================================*/
/*                         DEBUG PRINT MACRO                                  */
/*============================================================================*/
#ifdef W5500_DEBUG
#define DEBUG_PRINT(...)  printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/*============================================================================*/
/*                         PRIVATE CONSTANTS                                  */
/*============================================================================*/
#define DHCP_MAX_RETRY_COUNT    5

/*============================================================================*/
/*                         PRIVATE VARIABLES                                  */
/*============================================================================*/
static uint8_t dhcp_retry = 0;
static bool ip_assigned_flag = false;

static uint8_t dhcp_buffer[548];
static uint8_t dhcp_socket = ETH_CONFIG_DHCP_SOCKET;

/*============================================================================*/
/*                         CALLBACKS FROM DHCP LIBRARY                        */
/*============================================================================*/

/**
 * @brief Called when IP is successfully assigned or renewed.
 */
static void on_dhcp_assigned(void)
{
    DEBUG_PRINT("[DHCP] IP assigned.\r\n");

    getIPfromDHCP(g_network_info.ip);
    getGWfromDHCP(g_network_info.gw);
    getSNfromDHCP(g_network_info.sn);
    getDNSfromDHCP(g_network_info.dns);
    g_network_info.dhcp = NETINFO_DHCP;

    eth_config_set_netinfo(&g_network_info);
    ip_assigned_flag = true;
}

/**
 * @brief Called when a conflict is detected on the assigned IP.
 */
static void on_dhcp_conflict(void)
{
    DEBUG_PRINT("[DHCP] IP conflict detected.\r\n");
    w5500_dhcp_stop();
    ip_assigned_flag = false;
}

/*============================================================================*/
/*                         PUBLIC API IMPLEMENTATION                          */
/*============================================================================*/

void w5500_dhcp_init(void)
{
    DHCP_init(dhcp_socket, dhcp_buffer);
    reg_dhcp_cbfunc(on_dhcp_assigned, on_dhcp_assigned, on_dhcp_conflict);
    ip_assigned_flag = false;
    dhcp_retry = 0;

    DEBUG_PRINT("[DHCP] DHCP client initialized on socket %d.\r\n", dhcp_socket);
}

void w5500_dhcp_task1000ms(void)
{
    DHCP_time_handler();
}

uint8_t w5500_dhcp_task10ms(void)
{
    uint8_t dhcp_state = DHCP_run();

    if (!ip_assigned_flag) {
        switch (dhcp_state) {
            case DHCP_IP_ASSIGN:
            case DHCP_IP_CHANGED:
                DEBUG_PRINT("[DHCP] New IP acquired.\r\n");
                break;

            case DHCP_FAILED:
                if (++dhcp_retry > DHCP_MAX_RETRY_COUNT) {
                    DEBUG_PRINT("[DHCP] Max retry exceeded.\r\n");
                    w5500_dhcp_stop();
                }
                break;
        }
    }

    return dhcp_state;
}

void w5500_dhcp_stop(void)
{
    DHCP_stop();
    ip_assigned_flag = false;
    DEBUG_PRINT("[DHCP] DHCP client stopped.\r\n");
}

void w5500_getInfo(void)
{
    wiz_NetInfo net;
    eth_config_get_netinfo(&net);

    printf("--- W5500 Network Information (from DHCP) ---\r\n");
    printf("  IP Address: %d.%d.%d.%d\r\n", net.ip[0], net.ip[1], net.ip[2], net.ip[3]);
    printf("  Gateway:    %d.%d.%d.%d\r\n", net.gw[0], net.gw[1], net.gw[2], net.gw[3]);
    printf("  Subnet Mask:%d.%d.%d.%d\r\n", net.sn[0], net.sn[1], net.sn[2], net.sn[3]);
    printf("  DNS Server: %d.%d.%d.%d\r\n", net.dns[0], net.dns[1], net.dns[2], net.dns[3]);
    printf("  DHCP Mode:  %s\r\n", net.dhcp == NETINFO_DHCP ? "DHCP" : "STATIC");
    printf("  Lease Time: %lu seconds\r\n", getDHCPLeasetime());
    printf("---------------------------------------------\r\n");
}
