/**
 * @file w25q128.h
 * @brief Driver for W25Q128JVSIQ external SPI flash
 *
 * @details Supports read, write, and sector erase using STM32 HAL SPI with CMSIS-RTOS2 mutex protection
 */

#ifndef W25Q128_H
#define W25Q128_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "cmsis_os2.h"

/* W25Q128 Command Set */
#define W25_CMD_READ_DATA            0x03
#define W25_CMD_FAST_READ            0x0B
#define W25_CMD_PAGE_PROGRAM         0x02
#define W25_CMD_SECTOR_ERASE         0x20
#define W25_CMD_BLOCK32K_ERASE       0x52
#define W25_CMD_BLOCK64K_ERASE       0xD8
#define W25_CMD_CHIP_ERASE           0xC7
#define W25_CMD_READ_STATUS1         0x05
#define W25_CMD_READ_STATUS2         0x35
#define W25_CMD_WRITE_ENABLE         0x06
#define W25_CMD_WRITE_DISABLE        0x04
#define W25_CMD_READ_ID              0x9F

/* Status Register Bits */
#define W25_STATUS1_BUSY             0x01
#define W25_STATUS1_WEL              0x02

/* Flash Geometry */
#define W25_PAGE_SIZE                256        /* 256 bytes per page */
#define W25_SECTOR_SIZE              4096       /* 4KB sector size */
#define W25_BLOCK32K_SIZE            0x8000     /* 32KB block size */
#define W25_BLOCK64K_SIZE            0x10000    /* 64KB block size */
#define W25_FLASH_SIZE               0x1000000  /* 16MB total capacity */

/* Expected JEDEC ID values */
#define W25_MANUFACTURER_ID          0xEF       /* Winbond */
#define W25_MEMORY_TYPE              0x40       /* W25Q series */
#define W25_CAPACITY_ID              0x18       /* 128Mbit */

/* Externally defined SPI interface */
extern SPI_HandleTypeDef hspi1;
#define W25_SPI_HANDLE               hspi1
#define W25_CS_GPIO_PORT             GPIOA
#define W25_CS_GPIO_PIN              GPIO_PIN_4
#define W25_CS_LOW()                 HAL_GPIO_WritePin(W25_CS_GPIO_PORT, W25_CS_GPIO_PIN, GPIO_PIN_RESET)
#define W25_CS_HIGH()                HAL_GPIO_WritePin(W25_CS_GPIO_PORT, W25_CS_GPIO_PIN, GPIO_PIN_SET)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the flash driver and create mutex
 * @return true if initialization successful, false otherwise
 */
bool w25q128_init(void);

/**
 * @brief Read the flash JEDEC ID
 * @param id_buf Buffer to store the 3-byte ID (Manufacturer, Memory Type, Capacity)
 * @return true if successful, false otherwise
 */
bool w25q128_read_id(uint8_t *id_buf);

/**
 * @brief Read data from flash memory
 * @param addr Start address to read from
 * @param buf Buffer to store read data
 * @param len Number of bytes to read
 * @return true if successful, false otherwise
 */
bool w25q128_read_bytes(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief Write data to a single page (max 256 bytes)
 * @param addr Start address to write to (should be page-aligned for best performance)
 * @param data Data buffer to write
 * @param len Number of bytes to write (max 256)
 * @return true if successful, false otherwise
 */
bool w25q128_write_page(uint32_t addr, const uint8_t *data, uint32_t len);

/**
 * @brief Erase a 4KB sector
 * @param addr Address within the sector to erase
 * @return true if successful, false otherwise
 */
bool w25q128_erase_sector(uint32_t addr);

/**
 * @brief Wait until flash is not busy or timeout occurs
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return true if flash ready, false if timed out
 */
bool w25q128_wait_ready(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* W25Q128_H */