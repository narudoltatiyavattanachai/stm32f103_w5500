/**
 * @file    w5500_icmp.h
 * @brief   ICMP Ping Task API Header for W5500 Ethernet Controller
 * @author  Narudol T.
 * @date    2025-06-10
 */

 #ifndef W5500_ICMP_H
 #define W5500_ICMP_H
 
 #include <stdint.h>
 
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
 void w5500_icmp_task10ms(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* W5500_ICMP_H */