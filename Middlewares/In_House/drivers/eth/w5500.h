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

#ifndef _W5500_H_
#define _W5500_H_

#include <stdbool.h>
#include <stdint.h>

/* STM32 HAL includes */
#include "stm32f1xx_hal.h"

/* WIZnet driver includes */
#include "wizchip_conf.h"

/* ==========================================================================
 * W5500 HARDWARE INTERFACE FUNCTIONS
 * These functions provide the hardware-level access to the W5500 controller
 * ==========================================================================*/

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
bool w5500_init(SPI_HandleTypeDef *spi_handle, const uint8_t mac[6], const uint8_t ip[4], 
                const uint8_t subnet[4], const uint8_t gateway[4]);

/**
 * @brief Assert chip select (CS) pin to select W5500 for communication
 * @note  This is used by the WIZnet driver library for SPI control
 */
void w5500_select(void);

/**
 * @brief De-assert chip select (CS) pin to deselect W5500
 * @note  This is used by the WIZnet driver library for SPI control
 */
void w5500_deselect(void);

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
uint8_t w5500_spi_read_write(uint8_t data);

/**
 * @brief Receive multiple bytes over SPI
 * @param buf Buffer to store received data
 * @param len Number of bytes to receive
 */
void w5500_spi_read_burst(uint8_t *buf, uint16_t len);

/**
 * @brief Transmit multiple bytes over SPI
 * @param buf Buffer containing data to send
 * @param len Number of bytes to transmit
 */
void w5500_spi_write_burst(uint8_t *buf, uint16_t len);

/**
 * @brief Calculate IP/ICMP checksum
 * @param data Pointer to data buffer
 * @param len Length of data in bytes
 * @return Calculated checksum
 */
uint16_t w5500_calc_checksum(const uint8_t *data, uint16_t len);

/**
 * @brief Restart the W5500 hardware
 * 
 * @details Performs a hardware reset by toggling the reset pin
 *          Can be called to recover from error conditions
 * 
 * @return true if reset was successful
 */
bool w5500_restart(void);

/**
 * @brief Check if the W5500 hardware is responding
 * 
 * @details Reads the chip version register to verify communication
 * 
 * @return true if hardware is detected and responding
 */
bool w5500_check_hardware(void);

/**
 * @brief Get the current link status
 * @return true if link is up, false if link is down
 */
bool w5500_get_link_status(void);

/**
 * @brief Get the current PHY speed
 * @return 0 for 10Mbps, 1 for 100Mbps
 */
uint8_t w5500_get_phy_speed(void);

/**
 * @brief Get the current PHY duplex mode
 * @return 0 for half-duplex, 1 for full-duplex
 */
uint8_t w5500_get_phy_duplex(void);

#endif /* _W5500_H_ */