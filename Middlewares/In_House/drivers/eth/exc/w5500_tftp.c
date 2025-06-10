/**
 * @file w5500_tftp.c
 * @brief TFTP client functionality for W5500 Ethernet controller
 * 
 * @details Implementation of the modular wrapper around the third-party TFTP client library
 *          for file transfer over the network using TFTP protocol.
 */

#include "w5500_tftp.h"
#include "w5500_socket.h"
#include "main.h"
#include <string.h>

/* Include the third-party TFTP library */
#include "../Third_Party/ioLibrary_Driver_v3.2.0/Internet/TFTP/tftp.h"

/* Enable debug prints if defined */
#ifdef W5500_DEBUG
#define TFTP_DEBUG(...) printf(__VA_ARGS__)
#else
#define TFTP_DEBUG(...) /* Not defined */
#endif

/* Configuration constants if not defined in eth_config.h */
#ifndef ETH_TFTP_SOCKET
#define ETH_TFTP_SOCKET     3  /**< Default socket for TFTP */
#endif

#ifndef ETH_TFTP_BUF_SIZE
#define ETH_TFTP_BUF_SIZE   1024  /**< Default TFTP buffer size */
#endif

/* Private variables ---------------------------------------------------------*/
static uint8_t tftp_socket = ETH_TFTP_SOCKET;  /* Socket for TFTP operations */
static uint8_t tftp_buffer[ETH_TFTP_BUF_SIZE]; /* Buffer for TFTP transfers */
static bool tftp_initialized = false;                /* Initialization flag */
static tftp_status_t tftp_current_status = TFTP_STATUS_PROGRESS;  /* Current transfer status */

/* Data callback function */
static void (*data_callback)(uint8_t *data, uint16_t data_len, uint16_t block_number) = NULL;

/**
 * @brief Data handler for received TFTP packets
 * 
 * This function is called when a TFTP data packet is received, providing the
 * packet data to the registered callback function.
 * 
 * @param data Pointer to received data
 * @param data_len Length of the data
 * @param block_number Block number in the transfer
 */
static void tftp_data_handler(uint8_t *data, uint16_t data_len, uint16_t block_number)
{
    /* Call the registered callback if available */
    if (data_callback != NULL) {
        data_callback(data, data_len, block_number);
    }
    
    /* If no callback is registered, we just discard the data */
    #ifdef F_STORAGE
    else {
        /* Use the implementation provided by the application if available */
        save_data(data, data_len, block_number);
    }
    #endif
}

/**
 * @brief Initialize TFTP client
 * @return true if successful, false otherwise
 */
bool w5500_tftp_init(void)
{
    /* Initialize the third-party TFTP client */
    TFTP_init(tftp_socket, tftp_buffer);
    
    TFTP_DEBUG("[TFTP] Initialized on socket %d with buffer size %d\r\n", 
              tftp_socket, ETH_TFTP_BUF_SIZE);
    
    /* Set initial status to PROGRESS */
    tftp_current_status = TFTP_STATUS_PROGRESS;
    
    tftp_initialized = true;
    return true;
}

/**
 * @brief Read a file from TFTP server
 * @param server_ip Server IP address (4 bytes)
 * @param filename Name of the file to read
 * @param mode Transfer mode (ASCII or BINARY)
 * @return TFTP_OK if request started successfully, error code otherwise
 */
tftp_result_t w5500_tftp_read_request(const uint8_t server_ip[4], const char *filename, tftp_mode_t mode)
{
    uint32_t ip_addr;
    
    /* Make sure TFTP is initialized */
    if (!tftp_initialized && !w5500_tftp_init()) {
        TFTP_DEBUG("[TFTP] Failed to initialize\r\n");
        return TFTP_ERROR_NOT_INITIALIZED;
    }
    
    /* Validate parameters */
    if (!server_ip || !filename) {
        TFTP_DEBUG("[TFTP] Invalid parameters\r\n");
        return TFTP_ERROR_INVALID;
    }
    
    /* Convert IP to uint32_t */
    ip_addr = (server_ip[0] << 24) | (server_ip[1] << 16) | (server_ip[2] << 8) | server_ip[3];
    
    /* Reset status */
    tftp_current_status = TFTP_STATUS_PROGRESS;
    
    /* Begin TFTP read request */
    TFTP_read_request(ip_addr, (uint8_t*)filename);
    
    TFTP_DEBUG("[TFTP] Read request sent for file '%s' to %d.%d.%d.%d\r\n",
              filename, server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
    
    return TFTP_OK;
}

/**
 * @brief Process TFTP operations (must be called periodically)
 * @return TFTP_OK if operation in progress, TFTP_ERROR codes if failed, positive value if completed
 */
tftp_result_t w5500_tftp_run(void)
{
    int result;
    
    /* Make sure TFTP is initialized */
    if (!tftp_initialized) {
        TFTP_DEBUG("[TFTP] Not initialized\r\n");
        return TFTP_ERROR_NOT_INITIALIZED;
    }
    
    /* Run the TFTP state machine */
    result = TFTP_run();
    
    /* Update status based on the progress state from TFTP library */
    switch (g_progress_state) {
        case TFTP_PROGRESS:
            tftp_current_status = TFTP_STATUS_PROGRESS;
            result = TFTP_OK;
            break;
            
        case TFTP_FAIL:
            tftp_current_status = TFTP_STATUS_FAILED;
            TFTP_DEBUG("[TFTP] Transfer failed\r\n");
            result = TFTP_ERROR_TRANSFER;
            break;
            
        case TFTP_SUCCESS:
            tftp_current_status = TFTP_STATUS_SUCCESS;
            TFTP_DEBUG("[TFTP] Transfer completed successfully\r\n");
            result = 1; /* Positive value indicates success */
            break;
    }
    
    return result;
}

/**
 * @brief Get current TFTP transfer status
 * @return Current transfer status (PROGRESS, FAILED, SUCCESS)
 */
tftp_status_t w5500_tftp_get_status(void)
{
    return tftp_current_status;
}

/**
 * @brief TFTP timer handler (must be called every 1 second)
 */
void w5500_tftp_timer_handler(void)
{
    if (tftp_initialized) {
        tftp_timeout_handler();
    }
}

/**
 * @brief Set TFTP socket number
 * @param socket Socket number to use
 * @return true if successful, false otherwise
 */
bool w5500_tftp_set_socket(uint8_t socket)
{
    /* Cannot change socket if already initialized */
    if (tftp_initialized) {
        TFTP_exit();
        tftp_initialized = false;
    }
    
    tftp_socket = socket;
    TFTP_DEBUG("[TFTP] Socket set to %d\r\n", tftp_socket);
    
    /* Re-initialize with new socket */
    return w5500_tftp_init();
}

/**
 * @brief Set TFTP timeout in seconds
 * @param timeout Timeout value in seconds
 */
void w5500_tftp_set_timeout(uint8_t timeout)
{
    /* Set timeout through the underlying TFTP library */
    set_tftp_timeout(timeout);
    TFTP_DEBUG("[TFTP] Timeout set to %d seconds\r\n", timeout);
}

/**
 * @brief Register a callback function to handle received file data
 * @param callback Function pointer to the callback (pass NULL to unregister)
 * @return true if successful, false otherwise
 */
bool w5500_tftp_register_data_callback(void (*callback)(uint8_t *data, uint16_t data_len, uint16_t block_number))
{
    data_callback = callback;
    return true;
}

/* Add save_data function for F_STORAGE support */
#ifdef F_STORAGE
void save_data(uint8_t *data, uint32_t data_len, uint16_t block_number)
{
    /* If we have a registered callback, use it */
    if (data_callback != NULL) {
        data_callback(data, data_len, block_number);
    } else {
        /* Default implementation: just log the data */
        TFTP_DEBUG("[TFTP] Received block #%d, %d bytes (no storage handler)\r\n", 
                  block_number, data_len);
    }
}
#endif
