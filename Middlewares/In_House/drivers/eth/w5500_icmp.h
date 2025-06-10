/**
 * @file    w5500_icmp.h
 * @brief   ICMP Ping Task API Header for W5500 Ethernet Controller
 * @author  Narudol T.
 * @date    2025-06-10
 */

 #ifndef W5500_ICMP_H
 #define W5500_ICMP_H
 
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


 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Initialize the ICMP ping task (UDP-based ping listener)
  */
 void w5500_icmp_init(void);
 
 /**
  * @brief Periodic ICMP task function (to be called every 10ms)
  */
 void w5500_icmp_task100ms(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* W5500_ICMP_H */
