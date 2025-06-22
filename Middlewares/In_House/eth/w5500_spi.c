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

#include "w5500_spi.h"
#include "main.h"

/* ==========================================================================
 * CONFIGURATION AND DEFINES
 * ==========================================================================*/

#define W5500_SPI_TIMEOUT      1000


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
    printf("After SELECT: %d\n", HAL_GPIO_ReadPin(W5500_CS_GPIO_Port, W5500_CS_Pin));
}

/**
 * @brief De-assert chip select (CS) pin to deselect W5500
 * @note chcked Narudol T.   2025-06-10
 */
void w5500_cs_deselect(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
    printf("After DESELECT: %d\n", HAL_GPIO_ReadPin(W5500_CS_GPIO_Port, W5500_CS_Pin));
}


uint8_t w5500_spi_read(void)
{
    uint8_t tx = 0x00;
    uint8_t rx = 0x00;
    if(HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, W5500_SPI_TIMEOUT) != HAL_OK) {
        printf("SPI transfer error!\n");
        //Error_Handler();
    }
    return rx;
}

void w5500_spi_readburst(uint8_t* pBuf, uint16_t len)
{
    // More efficient than looping: do whole burst at once
    uint8_t tx[len];
    memset(tx, 0, len);
    HAL_SPI_TransmitReceive(&hspi2, tx, pBuf, len, W5500_SPI_TIMEOUT);
}

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
void w5500_spi_write(uint8_t byte)
{
    HAL_SPI_Transmit(&hspi2, &byte, 1, W5500_SPI_TIMEOUT);
}

void w5500_spi_writeburst(uint8_t* pBuf, uint16_t len)
{
    HAL_SPI_Transmit(&hspi2, pBuf, len, W5500_SPI_TIMEOUT);
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
 * @brief Initialize the W5500 hardware and SPI communication
 * 
 * This function:
 *  - Registers SPI and chip select callbacks
 *  - Performs a hardware reset
 *  - Verifies SPI connection (VERSIONR == 0x04)
 *  - Initializes socket TX/RX buffers
 *  - Applies static IP configuration
 *  - Reads back the applied network info for verification
 */
void w5500_spi_init(void)
{
	printf("\n=== W5500 SPI Hardware Setup ===\n");

	// Re-Confirm SPI Setting CR1 & CR2
	printf("=== Check Current SPI2_CR1 ===\n");
	printf("CR1: 0x%04lX\n", SPI2->CR1);
	printf("=== Check Current SPI2_CR2 ===\n");
	printf("CR2: 0x%04lX\n", SPI2->CR2);

///////////////////////////////////////////////////////////////////////////////////////////////////
    // Optionally test raw SPI transfer
    printf("Testing raw SPI transfer...\n");
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
    uint8_t tx = 0xAA;
    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, 1000);
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
    printf("Raw SPI test done, RX = 0x%02X\n", rx);

    ///////////////////////////////////////////////////////////////////////////////////////////////////
        // Optionally version read
    uint8_t ver, hdr[3] = {0x00, 0x39, 0x80}; // Addr high, Addr low, Read control byte

    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, hdr, 3, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, &ver, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);

    printf("W5500 VERSIONR: 0x%02X\n", ver);

///////////////////////////////////////////////////////////////////////////////////////////////////
    // 7️⃣ Continue with W5500-specific initialization
    printf("\n=== W5500 Hardware Initialization ===\n");
    printf("Resetting W5500...\n");
    w5500_spi_reset();
    osDelay(10);

///////////////////////////////////////////////////////////////////////////////////////////////////
    printf("Registering chip select callbacks...\n");
    reg_wizchip_cs_cbfunc(w5500_cs_select, w5500_cs_deselect);

    printf("Registering SPI byte read/write callbacks...\n");
    reg_wizchip_spi_cbfunc(w5500_spi_read, w5500_spi_write);

    printf("Registering SPI burst read/write callbacks...\n");
    reg_wizchip_spiburst_cbfunc(w5500_spi_readburst, w5500_spi_writeburst);

    printf("Finished callbacks registeration...\n");
///////////////////////////////////////////////////////////////////////////////////////////////////

    printf("Initializing socket buffers...\n");
    // Use centralized buffer configuration from eth_config.h
    uint8_t rx_tx_buff_sizes[ETH_CONFIG_TOTAL_BUFFERS];
    for (int i = 0; i < ETH_CONFIG_TOTAL_BUFFERS; i++) {
        rx_tx_buff_sizes[i] = ETH_CONFIG_BUFFER_SIZE_KB;
    }
    if (wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes) != 0)
    {
        printf("ERROR: wizchip_init() failed! Aborting.\n");
        //Error_Handler();
    }

    printf("Applying static network configuration...\n");
    eth_config_init_static();
    eth_config_set_netinfo(&g_network_info);

    printf("=== W5500 Initialization Complete ===\n");
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
void w5500_spi_reset(void)
{
	HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
	osDelay(10); // At least 1ms = safe for PMODE latching
	HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
	osDelay(10); // Wait for internal init
}


/* ==========================================================================
 * NOTE: Socket connection functions have been moved to w5500_socket.c
 * Transport functions have been moved to w5500_uros.c
 * ==========================================================================*/
