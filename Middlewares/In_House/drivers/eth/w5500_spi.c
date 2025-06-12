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
    printf("CS LOW\r\n");   // In w5500_cs_select()
}

/**
 * @brief De-assert chip select (CS) pin to deselect W5500
 * @note chcked Narudol T.   2025-06-10
 */
void w5500_cs_deselect(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
    printf("CS HIGH\r\n");  // In w5500_cs_deselect()
}

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
uint8_t w5500_spi_read(void) {
    uint8_t tx = 0xFF, rx = 0;
    if (HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, W5500_SPI_TIMEOUT) != HAL_OK) {
        printf("SPI READ ERROR\r\n");
    } else {
        printf("SPI R: 0x%02X\r\n", rx);  // 🔍 Watch what's being read
    }
    return rx;
}

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
void w5500_spi_write(uint8_t byte) {
    HAL_SPI_Transmit(&hspi2, &byte, 1, W5500_SPI_TIMEOUT);
    printf("SPI W: 0x%02X\r\n", byte);  // 🔍 Watch what gets written
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
void w5500_spi_init(void) {
    printf("Initializing W5500...\r\n");
    __HAL_SPI_ENABLE(&hspi2);


    // 1. Register Chip Select callbacks (GPIO control)
    printf("Registering chip select callbacks...\r\n");
    reg_wizchip_cs_cbfunc(w5500_cs_select, w5500_cs_deselect);

    // 2. Register SPI byte-level read/write functions
    printf("Registering SPI byte read/write callbacks...\r\n");
    reg_wizchip_spi_cbfunc(w5500_spi_read, w5500_spi_write);

    // Optional: Burst mode SPI (if implemented)
    // reg_wizchip_spiburst_cbfunc(w5500_spi_read_burst, w5500_spi_write_burst);

    // 3. Perform hardware reset via reset pin
    printf("Resetting W5500 chip...\r\n");
    w5500_spi_reset();
    HAL_Delay(100);  // Give chip time to fully boot

    // 4. Check VERSIONR to confirm SPI communication
    uint8_t version = w5500_manual_read_version();
    printf("W5500 VERSIONR = 0x%02X\r\n", version);
    if (version != 0x04) {
        printf("❌ SPI test failed! W5500 not responding properly.\r\n");
        return;
    }
    printf("✅ W5500 VERSIONR = 0x04 (OK)\r\n");

    // 5. Initialize socket buffer sizes (2KB for socket 0)
    printf("Initializing socket buffers...\r\n");
    uint8_t txsize[8] = {2, 0, 0, 0, 0, 0, 0, 0};  // 2KB TX on socket 0
    uint8_t rxsize[8] = {2, 0, 0, 0, 0, 0, 0, 0};  // 2KB RX on socket 0

    if (wizchip_init(txsize, rxsize) != 0) {
        printf("❌ ctlwizchip(CW_INIT_WIZCHIP) failed\r\n");
        return;
    }

    // 6. Apply static IP configuration
    printf("Applying static network configuration...\r\n");
    eth_config_init_static();                     // Sets values in g_network_info
    // Add debug
    printf("Initialized g_network_info with static values:\r\n");
    printf("  MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           g_network_info.mac[0], g_network_info.mac[1], g_network_info.mac[2],
           g_network_info.mac[3], g_network_info.mac[4], g_network_info.mac[5]);
    printf("  IP  : %d.%d.%d.%d\r\n",
           g_network_info.ip[0], g_network_info.ip[1], g_network_info.ip[2], g_network_info.ip[3]);

    // ACTUAL apply to chip
    wizchip_setnetinfo(&g_network_info);  // Just call it directly


    // 7. Read back and verify applied network info
    printf("Reading back applied network configuration...\r\n");

    wiz_NetInfo current;
    wizchip_getnetinfo(&current);

    printf("W5500 Applied Config Check:\r\n");
    printf("  IP  : %d.%d.%d.%d\r\n", current.ip[0], current.ip[1], current.ip[2], current.ip[3]);
    printf("  MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           current.mac[0], current.mac[1], current.mac[2],
           current.mac[3], current.mac[4], current.mac[5]);

    uint8_t mac_read[6];
    getSHAR(mac_read);
    printf("  MAC Read via getSHAR(): %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           mac_read[0], mac_read[1], mac_read[2],
           mac_read[3], mac_read[4], mac_read[5]);
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
	HAL_Delay(2); // At least 1ms = safe for PMODE latching
	HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100); // Wait for internal init
}


/**
 * @brief Read the W5500 VERSIONR register manually (should return 0x04)
 * @return version value
 */
uint8_t w5500_manual_read_version(void)
{
    uint8_t cmd[3] = { 0x00, 0x39, 0x00 };  // Address 0x0039 | Read | Block 0
    uint8_t version = 0x00;

    w5500_cs_select();
    HAL_SPI_Transmit(&hspi2, cmd, 3, 100);
    HAL_SPI_Receive(&hspi2, &version, 1, 100);
    w5500_cs_deselect();

    printf("W5500 VERSIONR = 0x%02X\r\n", version);  // Expect 0x04
    return version;
}
void w5500_manual_test_write_config(void)
{
    // Test MAC and IP values

    uint8_t ip_addr[4]  = {192, 168, 68, 10};

    // === Write MAC to SHAR (0x0009)
    printf("[TEST] Writing SHAR...\r\n");
    w5500_cs_select();

    uint8_t shar_cmd[3] = {0x00, 0x09, 0x04};  // Write to Common Register Block
    HAL_SPI_Transmit(&hspi2, shar_cmd, 3, 100);

    uint8_t mac_addr[6] = {0x08, 0x28, 0x00, 0x01, 0x02, 0x03};
    HAL_SPI_Transmit(&hspi2, mac_addr, 6, 100);


    w5500_cs_deselect();

    HAL_Delay(5);  // Allow W5500 internal update



    // === Read MAC back
    printf("[TEST] Reading SHAR...\r\n");


    w5500_cs_select();

    uint8_t shar_read_cmd[3] = {0x00, 0x09, 0x00};  // Read from Common Register Block
    HAL_SPI_Transmit(&hspi2, shar_read_cmd, 3, 100);

    uint8_t mac_read[6] = {0};
    HAL_SPI_Receive(&hspi2, mac_read, 6, 100);

    w5500_cs_deselect();

    printf("SHAR Readback: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
           mac_read[0], mac_read[1], mac_read[2],
           mac_read[3], mac_read[4], mac_read[5]);

    // === Write IP to SIPR (0x000F)
    printf("[TEST] Writing SIPR...\r\n");
    w5500_cs_select();
    uint8_t sipr_cmd[3] = {0x00, 0x0F, 0x04};  // Write to SIPR
    HAL_SPI_Transmit(&hspi2, sipr_cmd, 3, 100);
    HAL_SPI_Transmit(&hspi2, ip_addr, 4, 100);
    w5500_cs_deselect();

    HAL_Delay(5);  // Wait for update

    // === Read IP back
    printf("[TEST] Reading SIPR...\r\n");
    uint8_t ip_read[4] = {0};
    w5500_cs_select();
    uint8_t sipr_read_cmd[3] = {0x00, 0x0F, 0x00};  // Read from SIPR
    HAL_SPI_Transmit(&hspi2, sipr_read_cmd, 3, 100);
    HAL_SPI_Receive(&hspi2, ip_read, 4, 100);
    w5500_cs_deselect();

    printf("SIPR Readback: %d.%d.%d.%d\r\n",
           ip_read[0], ip_read[1], ip_read[2], ip_read[3]);

    // Optional: print diff warning
    if (memcmp(mac_read, mac_addr, 6) != 0 || memcmp(ip_read, ip_addr, 4) != 0) {
        printf("❌ [FAIL] W5500 config write verification failed.\r\n");
    } else {
        printf("✅ [PASS] W5500 SHAR/SIPR write verified OK.\r\n");
    }
}

/* ==========================================================================
 * NOTE: Socket connection functions have been moved to w5500_socket.c
 * Transport functions have been moved to w5500_uros.c
 * ==========================================================================*/
