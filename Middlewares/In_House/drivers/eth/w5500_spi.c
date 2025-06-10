/**
 * @file w5500.c
 * @brief W5500 Ethernet hardware initialization implementation for STM32F103
 * 
 * @details This file implements the hardware-level access layer for the WIZnet W5500
 *          Ethernet controller on STM32F103 microcontrollers. The implementation
 *          focuses on:
 *          1. SPI interface initialization and communication
 *          2. Hardware reset and initialization sequence
 *          3. Register configuration and access
 *          4. Network parameter setup
 * 
 *          This module provides only the hardware interface required by socket
 *          and transport implementations in other modules.
 * 
 * @author Your Name
 * @date 2025-06-09
 * @version 1.0
 */

/* ========================================================================== 
 * INCLUDES
 * ==========================================================================*/

/* Standard library includes */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <cmsis_os2.h>

/* W5500 hardware interface header */
#include "w5500_spi.h"
#include "eth_config.h"

/* Hardware abstraction defines */
#include "main.h"  /* For GPIO pin definitions */

/* ==========================================================================
 * CONFIGURATION AND DEFINES
 * ==========================================================================*/

/**
 * @brief SPI communication timeout in milliseconds
 */
#define W5500_SPI_TIMEOUT      1000

/**
 * @brief Default socket buffer sizes in KB (2KB per socket)
 */
#define W5500_TX_BUFFER_SIZE   2
#define W5500_RX_BUFFER_SIZE   2


/* ==========================================================================
 * PRIVATE VARIABLES
 * ==========================================================================*/

extern SPI_HandleTypeDef hspi2;

/* ==========================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ==========================================================================*/

/* ==========================================================================
 * SPI INTERFACE FUNCTIONS
 * These are used by the wizchip driver for SPI communication
 * ==========================================================================*/

/**
 * @brief Assert chip select (CS) pin to select W5500 for communication
 * @note chcked Narudol T.   2025-06-10
 */
void w5500_cs_select(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief De-assert chip select (CS) pin to deselect W5500
 * @note chcked Narudol T.   2025-06-10
 */
void w5500_cs_deselect(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
uint8_t w5500_spi_read(void)
{
    uint8_t tx = 0xFF, rx;
    HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, W5500_SPI_TIMEOUT);
    return rx;
}

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
void w5500_spi_write(uint8_t byte) {
    HAL_SPI_Transmit(&hspi2, &byte, 1, W5500_SPI_TIMEOUT);
}

/* ==========================================================================
 * HARDWARE UTILITY FUNCTIONS
 * These functions provide utility operations for hardware control
 * ==========================================================================*/


/* ==========================================================================
 * PRIVATE HARDWARE FUNCTIONS
 * Internal functions for hardware initialization
 * ==========================================================================*/

/* ==========================================================================
 * PUBLIC API IMPLEMENTATION - HARDWARE FUNCTIONS
 * These functions provide the core W5500 hardware initialization
 * ==========================================================================*/

/**
 * @brief Initialize the W5500 hardware and configure network settings
 * 
 * @param spi_handle Pointer to STM32 HAL SPI handle for communication
 * @param mac MAC address for W5500 (6 bytes)
 * @param ip IP address for local device (4 bytes)
 * @param subnet Subnet mask (4 bytes) 
 * @param gateway Gateway IP address (4 bytes)
 * @return true if initialization successful, false otherwise
 */
void w5500_init(void) {

    // 1. Register Chip Select callbacks
    reg_wizchip_cs_cbfunc(w5500_cs_select, w5500_cs_deselect);

    // 2. Register SPI read/write (byte-based)
    reg_wizchip_spi_cbfunc(w5500_spi_read, w5500_spi_write);

    // 3. Optionally register burst read/write (recommended for speed)
    //reg_wizchip_spiburst_cbfunc(w5500_spi_read_burst, w5500_spi_write_burst);

    // 4. Reset the W5500 hardware
    w5500_reset();

    // 5. Initialize socket buffer sizes (1 socket, 2KB each)
    uint8_t txsize[1] = {2};
    uint8_t rxsize[1] = {2};
    if (wizchip_init(txsize, rxsize) != 0) {
        // Optionally log error or blink LED
        return;
    }

    // 6. Set network configuration (static IP)
    eth_config_init_static();            // Populate default static info
    eth_config_set_netinfo(&g_network_info);  // Apply to W5500

    // 7. (Optional) Verify the configuration
    wiz_NetInfo current;
    wizchip_getnetinfo(&current);
    // Compare current.ip, current.mac... (for debug/logging)
}

/* ==========================================================================
 * NOTE: Socket functions have been moved to w5500_socket.c module
 * 
 * The following functions were previously located here and are now in w5500_socket.c:
 * - w5500_socket_open(): Opens a socket and connects to a remote endpoint
 * - w5500_socket_close(): Closes an active socket connection
 * ==========================================================================*/

/* ==========================================================================
 * PRIVATE IMPLEMENTATION FUNCTIONS
 * ==========================================================================*/

/* ==========================================================================
 * NOTE: Socket and network initialization functions have been moved to the
 * appropriate modules (w5500_socket.c and w5500_uros.c)
 * ==========================================================================*/

/* ==========================================================================
 * HARDWARE UTILITY FUNCTIONS
 * Functions to reset the chip and verify hardware status
 * ==========================================================================*/

/**
 * @brief Restart the W5500 hardware
 * 
 * @details Performs a hardware reset by toggling the reset pin
 *          Can be called to recover from error conditions
 * 
 */
void w5500_reset(void)
{
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    osDelay(1); // ≥ 1 ms to ensure >500 µs
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    osDelay(100); // Wait for W5500 to initialize
}

/* ==========================================================================
 * NOTE: Socket connection functions have been moved to w5500_socket.c
 * Transport functions have been moved to w5500_uros.c
 * ==========================================================================*/
