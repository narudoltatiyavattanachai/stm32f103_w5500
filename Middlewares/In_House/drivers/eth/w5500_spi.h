/**
 * @file w5500.h
 * @brief W5500 Ethernet hardware initialization interface for STM32F103
 * 
 * @details This module provides the hardware-level initialization for the 
 *          WIZnet W5500 Ethernet controller on STM32F103. It handles:
 *          - SPI interface configuration and communication
 *          - Hardware reset sequence
 *          - Chip initialization and register access
 *          - Network configuration at hardware level
 *          This module focuses only on hardware initialization and configuration.
 * 
 * @author Your Name
 * @date 2025-06-09
 */

#ifndef _W5500_SPI_H_
#define _W5500_SPI_H_

#include <stdint.h> // For uint8_t
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h" // For HAL (if needed)

/* WIZnet driver includes */
#include "../../../Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/wizchip_conf.h"

/*============================================================================*/
/*                         W5500 HARDWARE PIN DEFINITIONS                     */
/*============================================================================*/

/* W5500 Ethernet Controller Pin Definitions */
#define W5500_RST_Pin        GPIO_PIN_13
#define W5500_RST_GPIO_Port  GPIOC

#define W5500_INT_Pin        GPIO_PIN_8
#define W5500_INT_GPIO_Port  GPIOA

#define W5500_CS_Pin         GPIO_PIN_12
#define W5500_CS_GPIO_Port   GPIOB

#define _WIZCHIP_            W5500
#define _WIZCHIP_IO_MODE_    _WIZCHIP_IO_MODE_SPI_VDM_



/* ==========================================================================
 * W5500 HARDWARE INTERFACE FUNCTIONS
 * These functions provide the hardware-level access to the W5500 controller
 * ==========================================================================*/

/**
 * @brief Assert chip select (CS) pin to select W5500 for communication
 * @note  This is used by the WIZnet driver library for SPI control
 * @note chcked Narudol T.   2025-06-10
 */
void w5500_cs_select(void);

/**
 * @brief De-assert chip select (CS) pin to deselect W5500
 * @note  This is used by the WIZnet driver library for SPI control
 * @note chcked Narudol T.   2025-06-10
 */
void w5500_cs_deselect(void);

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
uint8_t w5500_spi_read(void);

/**
 * @brief Transmit multiple bytes over SPI
 * @param data Buffer containing data to send
 * @param len Number of bytes to transmit
 */
void w5500_spi_write(uint8_t byte);

/**
 * @brief Initialize the W5500 hardware and network settings
 * 
 * @details Sets up the W5500 Ethernet controller with the provided network
 *          configuration. This function handles:
 *          - SPI interface initialization
 *          - W5500 hardware reset sequence
 *          - Chip initialization and verification
 *          - Network parameters configuration
 *          - Socket buffer allocation
 * 
 * @param spi_handle Pointer to STM32 HAL SPI handle for W5500 communication
 * @param mac MAC address for the W5500 (6 bytes)
 * @param ip IP address for the local device (4 bytes)
 * @param subnet Subnet mask (4 bytes)
 * @param gateway Gateway IP address (4 bytes)
 * @return true if initialization successful, false otherwise
 */
 void w5500_spi_init(void);

/**
 * @brief Restart the W5500 hardware
 * 
 * @details Performs a hardware reset by toggling the reset pin
 *          Can be called to recover from error conditions
 * 
 * @return true if reset was successful
 */
void w5500_spi_reset(void);


/**
 * @brief Read the W5500 VERSIONR register (should return 0x04)
 * @return 8-bit version register value (0x04 for W5500)
 */
uint8_t w5500_manual_read_version(void);

void w5500_manual_test_write_config(void);

#endif /* _W5500_SPI_H_ */
