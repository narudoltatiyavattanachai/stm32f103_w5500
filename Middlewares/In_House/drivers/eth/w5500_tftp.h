/**
 * @file w5500_tftp.h
 * @brief TFTP client functionality for W5500 Ethernet controller
 * 
 * @details Provides a modular wrapper around the third-party TFTP client library
 *          for file transfer over the network using TFTP protocol.
 * 
 * @note    Uses the socket defined by ETH_CONFIG_TFTP_SOCKET (default: 1).
 */

#ifndef _W5500_TFTP_H_
#define _W5500_TFTP_H_

#include <stdint.h>
#include <stdbool.h>
#include "eth_config.h"

/*============================================================================*/
/** @section TFTP CLIENT SERVICE
 *  @brief TFTP client for file transfer operations
 *  @details Handles TFTP file transfer operations (read/write).
 *           Socket defined by ETH_CONFIG_TFTP_SOCKET.
 *============================================================================*/

/**
 * @brief TFTP error/status codes
 */
typedef enum {
    TFTP_OK = 0,                   /**< Successful operation */
    TFTP_ERROR_TIMEOUT = -1,       /**< Operation timed out */
    TFTP_ERROR_SOCKET = -2,        /**< Socket error */
    TFTP_ERROR_TRANSFER = -3,      /**< Transfer error */
    TFTP_ERROR_FILE = -4,          /**< File error */
    TFTP_ERROR_INVALID = -5,       /**< Invalid parameters */
    TFTP_ERROR_NOT_INITIALIZED = -6 /**< TFTP client not initialized */
} tftp_result_t;

/**
 * @brief TFTP transfer modes
 */
typedef enum {
    TFTP_MODE_ASCII = 0,          /**< ASCII transfer mode */
    TFTP_MODE_BINARY = 1          /**< Binary/octet transfer mode */
} tftp_mode_t;

/**
 * @brief TFTP progress status
 */
typedef enum {
    TFTP_STATUS_PROGRESS = 0,     /**< Transfer in progress */
    TFTP_STATUS_FAILED = 1,       /**< Transfer failed */
    TFTP_STATUS_SUCCESS = 2       /**< Transfer completed successfully */
} tftp_status_t;

/**
 * @brief Initialize TFTP client
 * @return true if successful, false otherwise
 */
bool w5500_tftp_init(void);

/**
 * @brief Read a file from TFTP server
 * @param server_ip Server IP address (4 bytes)
 * @param filename Name of the file to read
 * @param mode Transfer mode (ASCII or BINARY)
 * @return TFTP_OK if request started successfully, error code otherwise
 */
tftp_result_t w5500_tftp_read_request(const uint8_t server_ip[4], const char *filename, tftp_mode_t mode);

/**
 * @brief Process TFTP operations (must be called periodically)
 * @return TFTP_OK if operation in progress, TFTP_ERROR codes if failed, positive value if completed
 */
tftp_result_t w5500_tftp_run(void);

/**
 * @brief Get current TFTP transfer status
 * @return Current transfer status (PROGRESS, FAILED, SUCCESS)
 */
tftp_status_t w5500_tftp_get_status(void);

/**
 * @brief TFTP timer handler (must be called every 1 second)
 */
void w5500_tftp_timer_handler(void);

/**
 * @brief Set TFTP socket number
 * @param socket Socket number to use
 * @return true if successful, false otherwise
 */
bool w5500_tftp_set_socket(uint8_t socket);

/**
 * @brief Set TFTP timeout in seconds
 * @param timeout Timeout value in seconds
 */
void w5500_tftp_set_timeout(uint8_t timeout);

/**
 * @brief Register a callback function to handle received file data
 * @param callback Function pointer to the callback (pass NULL to unregister)
 * @return true if successful, false otherwise
 */
bool w5500_tftp_register_data_callback(void (*callback)(uint8_t *data, uint16_t data_len, uint16_t block_number));

#endif /* _W5500_TFTP_H_ */
