/**
 * @file w25q128.c
 * @brief Implementation of W25Q128JVSIQ flash memory driver
 * 
 * This driver provides the core functionality for interfacing with the W25Q128JVSIQ
 * SPI flash memory. It supports read, write, erase, and status operations with thread safety.
 * 
 * @note All configuration parameters are centralized in flash_config.h

#include "w25q128.h"
#include "../../../Core/Inc/flash_config.h"

/* Thread safety protection */
static osMutexId_t flash_mutex;
static const osMutexAttr_t flash_mutex_attr = {
    .name = "flashMutex"
};

#define FLASH_LOCK()   osMutexAcquire(flash_mutex, FLASH_MUTEX_TIMEOUT)
#define FLASH_UNLOCK() osMutexRelease(flash_mutex)

/* Use standardized timeouts from central configuration */

/**
 * @brief Enable write operations on flash
 */
static void w25q128_write_enable(void) {
    uint8_t cmd = W25_CMD_WRITE_ENABLE;
    W25_CS_LOW();
    HAL_SPI_Transmit(&W25_SPI_HANDLE, &cmd, 1, HAL_MAX_DELAY);
    W25_CS_HIGH();
}

bool w25q128_wait_ready(uint32_t timeout_ms) {
    uint8_t cmd = W25_CMD_READ_STATUS1;
    uint8_t status;
    uint32_t tickstart = HAL_GetTick();

    do {
        W25_CS_LOW();
        HAL_SPI_Transmit(&W25_SPI_HANDLE, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(&W25_SPI_HANDLE, &status, 1, HAL_MAX_DELAY);
        W25_CS_HIGH();

        if (!(status & W25_STATUS1_BUSY)) return true;
    } while ((HAL_GetTick() - tickstart) < timeout_ms);

    return false;
}

bool w25q128_read_id(uint8_t *id_buf) {
    FLASH_LOCK();
    uint8_t cmd = W25_CMD_READ_ID;
    uint8_t dummy[3] = {0};
    W25_CS_LOW();
    HAL_SPI_Transmit(&W25_SPI_HANDLE, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&W25_SPI_HANDLE, dummy, 3, HAL_MAX_DELAY);
    HAL_SPI_Receive(&W25_SPI_HANDLE, id_buf, 3, HAL_MAX_DELAY);
    W25_CS_HIGH();
    FLASH_UNLOCK();
    return true;
}

bool w25q128_read_bytes(uint32_t addr, uint8_t *buf, uint32_t len) {
    FLASH_LOCK();
    uint8_t cmd[4] = {
        W25_CMD_READ_DATA,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr >> 0),
    };
    W25_CS_LOW();
    HAL_SPI_Transmit(&W25_SPI_HANDLE, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(&W25_SPI_HANDLE, buf, len, HAL_MAX_DELAY);
    W25_CS_HIGH();
    FLASH_UNLOCK();
    return true;
}

bool w25q128_write_page(uint32_t addr, const uint8_t *data, uint32_t len) {
    if (len > W25_PAGE_SIZE) return false;
    FLASH_LOCK();
    w25q128_write_enable();
    uint8_t cmd[4] = {
        W25_CMD_PAGE_PROGRAM,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr >> 0),
    };
    W25_CS_LOW();
    HAL_SPI_Transmit(&W25_SPI_HANDLE, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&W25_SPI_HANDLE, (uint8_t *)data, len, HAL_MAX_DELAY);
    W25_CS_HIGH();
    bool result = w25q128_wait_ready(FLASH_TIMEOUT_WRITE);
    FLASH_UNLOCK();
    return result;
}

bool w25q128_erase_sector(uint32_t addr) {
    FLASH_LOCK();
    w25q128_write_enable();
    uint8_t cmd[4] = {
        W25_CMD_SECTOR_ERASE,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr >> 0),
    };
    W25_CS_LOW();
    HAL_SPI_Transmit(&W25_SPI_HANDLE, cmd, 4, HAL_MAX_DELAY);
    W25_CS_HIGH();
    bool result = w25q128_wait_ready(FLASH_TIMEOUT_ERASE);
    FLASH_UNLOCK();
    return result;
}

bool w25q128_init(void) {
    flash_mutex = osMutexNew(&flash_mutex_attr);
    if (flash_mutex == NULL) return false;

    uint8_t id[3];
    return w25q128_read_id(id) && (id[0] == 0xEF); // Winbond JEDEC ID
}
    