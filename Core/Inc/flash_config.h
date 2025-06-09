/**
 * @file flash_config.h
 * @brief W25Q128JVSIQ Flash Memory Configuration and Layout Definition
 * 
 * This file serves as the central configuration point for the W25Q128JVSIQ flash memory
 * driver and related modules. It defines:
 * 
 * 1. Memory layout for the 16MB W25Q128JVSIQ flash chip
 * 2. Centralized macros and constants for flash operations
 * 3. Interface references for the driver modules
 * 4. Status and error codes for flash operations
 * 
 * The flash is divided into the following sections:
 * - BOOT: Factory bootloader (protected)
 * - FIRMWARE: Three firmware slots (A=active, B=update target, C=fallback)
 * - META: OTA metadata with redundancy for reliability
 * - CONFIG: Device configuration parameters
 * - EEPROM: EEPROM emulation area with wear leveling
 * - LOG: Circular logging area
 * - USER: User data storage
 * - RESERVED: Reserved for future use
 * 
 * All flash driver modules in Middlewares/In_House/drivers/flash/ use this file
 * for configuration, ensuring consistent behavior and organization.
 */

#ifndef FLASH_CONFIG_H
#define FLASH_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "../../Middlewares/In_House/drivers/flash/w25q128.h"

/*---------------------------------------------------------------------------*/
/* Flash Operation Configuration Parameters                                   */
/*---------------------------------------------------------------------------*/

/* Debug configuration */
#define FLASH_DEBUG_ENABLED       0     /**< Set to 1 to enable debug messages */

/* Operation timeouts (milliseconds) */
#define FLASH_TIMEOUT_READ        100   /**< Read operation timeout */
#define FLASH_TIMEOUT_WRITE       500   /**< Write operation timeout */
#define FLASH_TIMEOUT_ERASE       5000  /**< Sector erase timeout */
#define FLASH_TIMEOUT_BLOCK_ERASE 30000 /**< Block erase timeout */

/* Retry parameters */
#define FLASH_MAX_RETRIES         3     /**< Maximum operation retry attempts */
#define FLASH_RETRY_DELAY_MS      10    /**< Delay between retries (ms) */

/* Buffer sizes */
#define FLASH_EEPROM_BUFFER_SIZE  256   /**< EEPROM operation buffer size */
#define FLASH_LOG_BUFFER_SIZE     512   /**< Logging buffer size */
#define FLASH_META_BUFFER_SIZE    128   /**< Metadata buffer size */

/* Thread safety */
#define FLASH_USE_MUTEX           1     /**< Use mutex for thread safety */
#define FLASH_MUTEX_TIMEOUT       1000  /**< Mutex acquisition timeout */

/*---------------------------------------------------------------------------*/
/* W25Q128 Flash Characteristics                                            */
/*---------------------------------------------------------------------------*/

/* Use W25Q128's constants directly from the driver */
#define FLASH_TOTAL_SIZE      W25_FLASH_SIZE        /* 16MB (128Mbit) */
#define FLASH_SECTOR_SIZE     W25_SECTOR_SIZE       /* 4KB sector */
#define FLASH_BLOCK32K_SIZE   W25_BLOCK32K_SIZE     /* 32KB block */
#define FLASH_BLOCK64K_SIZE   W25_BLOCK64K_SIZE     /* 64KB block */
#define FLASH_PAGE_SIZE       W25_PAGE_SIZE        /* 256-byte page (write unit) */

/*---------------------------------------------------------------------------*/
/* Firmware Storage - 3MB total                                             */
/*---------------------------------------------------------------------------*/

/* Base address for firmware storage */
#define FIRMWARE_BASE_ADDR    0x000000UL

/* Factory bootloader - 256KB, protected from overwrite */
#define BOOT_ADDR             (FIRMWARE_BASE_ADDR)
#define BOOT_SIZE             (256UL * 1024)

/* Firmware slot size - 768KB per slot to accommodate larger firmware */
#define FW_SLOT_SIZE          (768UL * 1024)

/* Three firmware slots (current, update target, and fallback) */
#define FW_SLOT_A_ADDR        (BOOT_ADDR + BOOT_SIZE)              /* 0x040000 - Active firmware */
#define FW_SLOT_B_ADDR        (FW_SLOT_A_ADDR + FW_SLOT_SIZE)      /* 0x100000 - OTA update target */
#define FW_SLOT_C_ADDR        (FW_SLOT_B_ADDR + FW_SLOT_SIZE)      /* 0x1C0000 - Fallback image */

/*---------------------------------------------------------------------------*/
/* Metadata Storage - 128KB total with redundancy                           */
/*---------------------------------------------------------------------------*/

/* Metadata region base */
#define META_BASE_ADDR        0x280000UL

/* Each copy is 32KB (aligned to erase blocks for reliability) */
#define META_COPY_SIZE        (32UL * 1024)

/* Duplicate metadata copies for redundancy */
#define META_COPY1_ADDR       (META_BASE_ADDR)
#define META_COPY2_ADDR       (META_COPY1_ADDR + META_COPY_SIZE)
#define META_COPY3_ADDR       (META_COPY2_ADDR + META_COPY_SIZE)

/* Metadata block structure offsets */
#define META_MAGIC_OFFSET     0           /* Magic number to validate metadata */
#define META_VERSION_OFFSET   4           /* Metadata format version */
#define META_FW_VERSION_OFFSET 8         /* Firmware version information */
#define META_ACTIVE_SLOT_OFFSET 16       /* Currently active firmware slot */
#define META_CRC_OFFSET       20         /* CRC of firmware slots */

/*---------------------------------------------------------------------------*/
/* Configuration Storage - 256KB                                             */
/*---------------------------------------------------------------------------*/

#define CONFIG_BASE_ADDR      0x2C0000UL
#define CONFIG_SIZE           (256UL * 1024)
#define CONFIG_COPY1_ADDR     (CONFIG_BASE_ADDR)
#define CONFIG_COPY2_ADDR     (CONFIG_BASE_ADDR + (CONFIG_SIZE / 2))

/*---------------------------------------------------------------------------*/
/* EEPROM Emulation - 512KB with wear leveling                              */
/*---------------------------------------------------------------------------*/

#define EEPROM_BASE_ADDR      0x300000UL
#define EEPROM_SIZE           (512UL * 1024)

/* EEPROM emulation is divided into sectors for wear leveling */
#define EEPROM_SECTOR_COUNT   (EEPROM_SIZE / FLASH_SECTOR_SIZE)
#define EEPROM_HEADER_SIZE    8           /* Bytes for sector header (counter, status) */

/*---------------------------------------------------------------------------*/
/* Logging Area - 1MB circular buffer                                        */
/*---------------------------------------------------------------------------*/

#define LOG_BASE_ADDR         0x380000UL
#define LOG_SIZE              (1UL * 1024 * 1024)
#define LOG_HEADER_SIZE       16          /* Log header size (timestamp, type, etc.) */

/*---------------------------------------------------------------------------*/
/* User Data Storage - 8MB                                                   */
/*---------------------------------------------------------------------------*/

#define USER_DATA_BASE_ADDR   0x480000UL
#define USER_DATA_SIZE        (8UL * 1024 * 1024)

/*---------------------------------------------------------------------------*/
/* Reserved Area - 3MB for future expansion                                  */
/*---------------------------------------------------------------------------*/

#define RESERVED_BASE_ADDR    0xC80000UL
#define RESERVED_SIZE         (3UL * 1024 * 1024)

/*---------------------------------------------------------------------------*/
/* Flash Management Macros                                                   */
/*---------------------------------------------------------------------------*/

/* Convert address to sector number */
#define ADDR_TO_SECTOR(addr)  ((addr) / FLASH_SECTOR_SIZE)

/* Convert address to 64KB block number */
#define ADDR_TO_BLOCK64(addr) ((addr) / FLASH_BLOCK64K_SIZE)

/* Align address to sector boundary (for erase operations) */
#define SECTOR_ALIGN(addr)    ((addr) & ~(FLASH_SECTOR_SIZE - 1))

/* Align address to 64KB block boundary */
#define BLOCK64_ALIGN(addr)   ((addr) & ~(FLASH_BLOCK64K_SIZE - 1))

/* Check if address is sector-aligned */
#define IS_SECTOR_ALIGNED(addr) (((addr) % FLASH_SECTOR_SIZE) == 0)

/* Magic number to validate metadata */
#define FLASH_META_MAGIC      0xA5C33CA5UL

/* Current metadata version */
#define FLASH_META_VERSION    0x0001

/*---------------------------------------------------------------------------*/
/* Flash Driver Status Codes                                                 */
/*---------------------------------------------------------------------------*/

/**
 * @brief Flash operation status codes
 */
typedef enum {
    FLASH_STATUS_OK = 0,            /**< Operation successful */
    FLASH_STATUS_BUSY = 1,           /**< Flash is busy */
    FLASH_STATUS_ERROR = -1,         /**< General error */
    FLASH_STATUS_TIMEOUT = -2,       /**< Operation timed out */
    FLASH_STATUS_INVALID_PARAM = -3, /**< Invalid parameter */
    FLASH_STATUS_PROTECTED = -4,     /**< Flash area is protected */
    FLASH_STATUS_NOT_ALIGNED = -5,   /**< Address not aligned properly */
    FLASH_STATUS_CRC_ERROR = -6,     /**< Data integrity check failed */
    FLASH_STATUS_NO_MEMORY = -7      /**< Insufficient memory for operation */
} flash_status_t;

/*---------------------------------------------------------------------------*/
/* Flash Operation Module References                                         */
/*---------------------------------------------------------------------------*/

/**
 * @brief External references to flash module initialization functions
 * These should be called during system initialization in appropriate order
 */
extern bool w25q128_init(void);
extern bool w25q128_eeprom_init(void);
extern bool w25q128_log_init(void);
extern bool w25q128_meta_init(void);

#endif /* FLASH_CONFIG_H */

/* End of flash_config.h */
