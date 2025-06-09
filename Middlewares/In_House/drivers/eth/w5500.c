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

/* W5500 hardware interface header */
#include "w5500.h"

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

/**
 * @brief Debug print macro (disabled by default)
 * @note  Uncomment and implement to enable debug printing
 */
#define DEBUG_PRINT(...)       // printf(__VA_ARGS__)

/* ==========================================================================
 * PRIVATE VARIABLES
 * ==========================================================================*/

/**
 * @brief Pointer to SPI handle used for W5500 communication
 */
static SPI_HandleTypeDef *hw_spi = NULL;

/* ==========================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ==========================================================================*/

/**
 * @brief Initialize W5500 hardware (SPI interface and reset pin)
 * @return true if successful, false otherwise
 */
static bool w5500_hw_init(void);

/**
 * @brief Hardware reset sequence for the W5500 controller
 * @return true if reset was successful, false otherwise
 */
static bool w5500_hw_reset(void);

/**
 * @brief Configure socket buffers for the W5500 controller
 * @return true if configuration successful, false otherwise
 */
static bool w5500_configure_buffers(void);

/**
 * @brief Set SPI callback functions for wizchip driver
 */
static void w5500_register_spi_functions(void);

/* ==========================================================================
 * SPI INTERFACE FUNCTIONS
 * These are used by the wizchip driver for SPI communication
 * ==========================================================================*/

/**
 * @brief Assert chip select (CS) pin to select W5500 for communication
 */
void w5500_select(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief De-assert chip select (CS) pin to deselect W5500
 */
void w5500_deselect(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief Exchange a single byte over SPI
 * @param data Byte to send
 * @return Byte received
 */
uint8_t w5500_spi_read_write(uint8_t data)
{
    uint8_t rx_data;
    
    /* Exchange a byte using HAL SPI transmit/receive */
    HAL_SPI_TransmitReceive(hw_spi, &data, &rx_data, 1, W5500_SPI_TIMEOUT);
    
    return rx_data;
}

/**
 * @brief Receive multiple bytes over SPI
 * @param buf Buffer to store received data
 * @param len Number of bytes to receive
 */
void w5500_spi_read_burst(uint8_t *buf, uint16_t len)
{
    /* Receive multiple bytes using HAL SPI receive */
    HAL_SPI_Receive(hw_spi, buf, len, W5500_SPI_TIMEOUT);
}

/**
 * @brief Transmit multiple bytes over SPI
 * @param buf Buffer containing data to send
 * @param len Number of bytes to transmit
 */
void w5500_spi_write_burst(uint8_t *buf, uint16_t len)
{
    /* Transmit multiple bytes using HAL SPI transmit */
    HAL_SPI_Transmit(hw_spi, buf, len, W5500_SPI_TIMEOUT);
}

/* ==========================================================================
 * HARDWARE UTILITY FUNCTIONS
 * These functions provide utility operations for hardware control
 * ==========================================================================*/

/**
 * @brief Calculate IP/ICMP checksum
 * @param data Pointer to data buffer
 * @param len Length of data in bytes
 * @return Calculated checksum
 */
uint16_t w5500_calc_checksum(const uint8_t *data, uint16_t len)
{
    uint32_t sum = 0;
    uint16_t i;
    
    /* Add all 16-bit words */
    for (i = 0; i < len - 1; i += 2) {
        sum += (data[i] << 8) | data[i + 1];
    }
    
    /* Add last byte if length is odd */
    if (len & 1) {
        sum += data[len - 1] << 8;
    }
    
    /* Add carry bits */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    /* Return one's complement */
    return (uint16_t)(~sum);
}


/* ==========================================================================
 * PRIVATE HARDWARE FUNCTIONS
 * Internal functions for hardware initialization
 * ==========================================================================*/

/**
 * @brief Perform hardware reset for W5500 chip
 * @return true if reset successful, false otherwise
 */
static bool w5500_hw_reset(void)
{
    /* Pull reset pin LOW for hardware reset */
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    OS_Delay(10); /* Hold reset for 10ms */
    
    /* Release reset pin */
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    OS_Delay(50); /* Wait for chip to initialize after reset */
    
    DEBUG_PRINT("W5500 hardware reset performed\r\n");
    return true;
}

/**
 * @brief Configure socket buffer sizes for the W5500
 * @return true if configuration successful
 */
static bool w5500_configure_buffers(void)
{
    /* Configure socket buffer sizes (2KB per socket) */
    uint8_t tx_mem_conf[8] = {W5500_TX_BUFFER_SIZE, W5500_TX_BUFFER_SIZE, 
                             W5500_TX_BUFFER_SIZE, W5500_TX_BUFFER_SIZE,
                             W5500_TX_BUFFER_SIZE, W5500_TX_BUFFER_SIZE,
                             W5500_TX_BUFFER_SIZE, W5500_TX_BUFFER_SIZE};
    uint8_t rx_mem_conf[8] = {W5500_RX_BUFFER_SIZE, W5500_RX_BUFFER_SIZE,
                             W5500_RX_BUFFER_SIZE, W5500_RX_BUFFER_SIZE,
                             W5500_RX_BUFFER_SIZE, W5500_RX_BUFFER_SIZE,
                             W5500_RX_BUFFER_SIZE, W5500_RX_BUFFER_SIZE};
                             
    /* Apply buffer size configuration */
    wizchip_setbufsize(tx_mem_conf, rx_mem_conf);
    
    DEBUG_PRINT("W5500 socket buffers configured\r\n");
    return true;
}

/* Register SPI callback function is defined above */

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
bool w5500_init(SPI_HandleTypeDef *spi_handle, const uint8_t mac[6], const uint8_t ip[4], 
                const uint8_t subnet[4], const uint8_t gateway[4])
{
    /* Network information structure */
    wiz_NetInfo net_info = {0};
    
    /* Store SPI handle for later use */
    hw_spi = spi_handle;
    if (hw_spi == NULL) {
        DEBUG_PRINT("W5500 initialization failed: Invalid SPI handle\r\n");
        return false;
    }
    
    /* Step 1: Perform hardware reset */
    if (!w5500_hardware_reset()) {
        DEBUG_PRINT("W5500 hardware reset failed\r\n");
        return false;
    }
    
    /* Step 2: Register SPI and chip select callback functions */
    w5500_register_spi_functions();
    
    /* Step 3: Initialize W5500 chip registers */
    wizchip_init(NULL, NULL);
    
    /* Step 4: Verify chip is accessible by checking version register */
    if (getVERSIONR() != 0x04) /* W5500 version is 0x04 */
    {
        DEBUG_PRINT("W5500 not detected or incorrect version\r\n");
        return false;
    }
    
    /* Step 5: Configure socket buffer sizes */
    if (!w5500_configure_buffers()) {
        DEBUG_PRINT("W5500 buffer configuration failed\r\n");
        return false;
    }
    
    /* Step 6: Configure network parameters */
    memcpy(net_info.mac, mac, 6);
    memcpy(net_info.ip, ip, 4);
    memcpy(net_info.sn, subnet, 4);
    memcpy(net_info.gw, gateway, 4);
    
    /* Apply network configuration */
    wizchip_setnetinfo(&net_info);
    
    DEBUG_PRINT("W5500 initialized successfully\r\n");
    return true;
}

/* ==========================================================================
 * NOTE: Socket functions have been moved to w5500_socket.c module
 * 
 * The following functions were previously located here and are now in w5500_socket.c:
 * - w5500_socket_open(): Opens a socket and connects to a remote endpoint
 * - w5500_socket_close(): Closes an active socket connection
 * ==========================================================================*/

/**
 * @brief Get the current link status
 * @return true if link is up, false if link is down
 */
bool w5500_get_link_status(void)
{
    uint8_t phy_status;
    
    /* Read the PHY status register */
    phy_status = getPHYCFGR();
    
    /* Check if the link is up (bit 0 of PHY status) */
    return (phy_status & PHYCFGR_LNK_ON) ? true : false;
}

/**
 * @brief Get the current PHY speed
 * @return 0 for 10Mbps, 1 for 100Mbps
 */
uint8_t w5500_get_phy_speed(void)
{
    uint8_t phy_status;
    
    /* Read the PHY status register */
    phy_status = getPHYCFGR();
    
    /* Check if running at 100Mbps (bit 1 of PHY status) */
    return (phy_status & PHYCFGR_SPD_100) ? 1 : 0;
}

/**
 * @brief Get the current PHY duplex mode
 * @return 0 for half-duplex, 1 for full-duplex
 */
uint8_t w5500_get_phy_duplex(void)
{
    uint8_t phy_status;
    
    /* Read the PHY status register */
    phy_status = getPHYCFGR();
    
    /* Check if running in full-duplex mode (bit 2 of PHY status) */
    return (phy_status & PHYCFGR_DPX_FULL) ? 1 : 0;
}

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
 * @return true if reset was successful
 */
bool w5500_restart(void)
{
    /* Perform hardware reset */
    return w5500_hardware_reset();
}

/**
 * @brief Check if the W5500 hardware is responding
 * 
 * @details Reads the chip version register to verify communication
 * 
 * @return true if hardware is detected and responding
 */
bool w5500_check_hardware(void)
{
    /* The W5500 should report version 0x04 */
    return (getVERSIONR() == 0x04);
}

/* ==========================================================================
 * NOTE: Socket connection functions have been moved to w5500_socket.c
 * Transport functions have been moved to w5500_uros.c
 * ==========================================================================*/