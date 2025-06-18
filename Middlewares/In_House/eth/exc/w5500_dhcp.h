/*============================================================================*/
/** @section DHCP CLIENT SERVICE (Socket 0)
 *  @brief DHCP initialization, periodic handler, and IP assignment
 *  @details All DHCP-related logic and state management for dynamic IP
 *           assignment. Socket defined by ETH_CONFIG_DHCP_SOCKET.
 *============================================================================*/
 #ifndef _W5500_DHCP_H_
 #define _W5500_DHCP_H_
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
#include <stdint.h> // For uint8_t
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h" // For HAL (if needed)

#include "w5500_spi.h"
#include "eth_config.h"
#include "../../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/socket.h"
#include "../../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/wizchip_conf.h"
#include "../../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/W5500/w5500.h"
#include "../../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Internet/DHCP/dhcp.h"


 // Define the socket number to be used for DHCP.
 // You might want to adjust this based on your W5500's socket allocation.
 #define W5500_DHCP_DEFAULT_SOCKET 0
 
 /**
  * @brief Initializes the W5500 DHCP client.
  * This function sets up the DHCP socket and prepares the client for
  * the DHCP negotiation process. It should be called once during system startup.
  * @return bool True if initialization was successful, false otherwise
  */
 bool w5500_dhcp_init(void);
 
 /**
  * @brief DHCP 1000ms (1 second) task handler.
  * This function should be called periodically every 1000ms (1 second)
  * to handle DHCP lease time tracking and renewal.
  */
 void w5500_dhcp_task1000ms(void);
 
 /**
  * @brief DHCP 10ms task handler.
  * This function should be called frequently, ideally every 10ms,
  * to process incoming DHCP messages and manage the DHCP state machine.
  * @return Returns the current DHCP state (e.g., DHCP_RUNNING, DHCP_IP_ASSIGN, etc.)
  * as defined in dhcp.h.
  */
 uint8_t w5500_dhcp_task10ms(void);
 
 /**
  * @brief Stops the W5500 DHCP client.
  * This function closes the DHCP socket and stops the DHCP processing.
  * To restart, call w5500_dhcp_init() and then continue calling w5500_dhcp_task10ms().
  */
 void w5500_dhcp_stop(void);
 
 /**
  * @brief Retrieves and prints the network information obtained via DHCP.
  * This includes the allocated IP address, Gateway, Subnet Mask, and DNS server.
  * Requires <stdio.h> for printing.
  */
 void w5500_getInfo(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif // _W5500_DHCP_H_
 
