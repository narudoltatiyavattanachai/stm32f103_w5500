/**
 * @file w5500_httpServer.h
 * @brief HTTP server functionality for W5500 Ethernet controller
 * 
 * @details Provides a modular wrapper around the third-party HTTP server library
 *          for serving web content over Ethernet.
 * 
 * @note    Uses the sockets defined in socket configuration.
 */

#ifndef _W5500_HTTPSERVER_H_
#define _W5500_HTTPSERVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "eth_config.h"

/*============================================================================*/
/** @section HTTP SERVER SERVICE
 *  @brief HTTP server for web-based configuration and monitoring
 *  @details Handles HTTP requests and serves web content.
 *============================================================================*/

/**
 * @brief HTTP server error/status codes
 */
typedef enum {
    HTTP_SERVER_OK = 0,                /**< Successful operation */
    HTTP_SERVER_ERROR_SOCKET = -1,     /**< Socket error */
    HTTP_SERVER_ERROR_BUFFER = -2,     /**< Buffer allocation error */
    HTTP_SERVER_ERROR_INIT = -3,       /**< Initialization error */
    HTTP_SERVER_ERROR_PARAM = -4       /**< Invalid parameter */
} http_server_result_t;

/**
 * @brief HTTP content types
 */
typedef enum {
    HTTP_CONTENT_HTML = 0,             /**< HTML content type */
    HTTP_CONTENT_CSS,                  /**< CSS content type */
    HTTP_CONTENT_JS,                   /**< JavaScript content type */
    HTTP_CONTENT_JSON,                 /**< JSON content type */
    HTTP_CONTENT_PNG,                  /**< PNG image type */
    HTTP_CONTENT_JPG,                  /**< JPG image type */
    HTTP_CONTENT_ICO,                  /**< ICO image type */
    HTTP_CONTENT_TEXT,                 /**< Plain text type */
    HTTP_CONTENT_BINARY                /**< Generic binary type */
} http_content_type_t;

/**
 * @brief HTTP server configuration structure
 */
typedef struct {
    uint16_t port;                     /**< HTTP server port, default: 80 */
    uint8_t max_sockets;               /**< Maximum number of sockets to use */
    uint16_t timeout;                  /**< Connection timeout in seconds */
    uint8_t *tx_buffer;                /**< Transmit buffer pointer */
    uint8_t *rx_buffer;                /**< Receive buffer pointer */
} http_server_config_t;

/**
 * @brief Initialize HTTP server with default settings
 * @return true if successful, false otherwise
 */
bool w5500_httpServer_init(void);

/**
 * @brief Initialize HTTP server with custom configuration
 * @param config Pointer to configuration structure
 * @return true if successful, false otherwise
 */
bool w5500_httpServer_init_with_config(const http_server_config_t *config);

/**
 * @brief Run the HTTP server (process connections) - call in main loop
 * @return HTTP_SERVER_OK if successful, error code otherwise
 */
http_server_result_t w5500_httpServer_run(void);

/**
 * @brief HTTP server timer handler - call every 1 second
 */
void w5500_httpServer_timer_handler(void);

/**
 * @brief Register in-memory web content (HTML, CSS, JS, etc.)
 * @param name Content name/URL path
 * @param content Pointer to the content data
 * @param content_len Length of the content
 * @param content_type Content type
 * @return true if successful, false otherwise
 */
bool w5500_httpServer_register_content(
    const char *name, 
    const uint8_t *content, 
    uint32_t content_len, 
    http_content_type_t content_type);

/**
 * @brief Register a callback to be called when MCU reset is requested via web interface
 * @param callback Function pointer to the callback
 */
void w5500_httpServer_register_reset_callback(void (*callback)(void));

/**
 * @brief Register a callback to be called for watchdog reset
 * @param callback Function pointer to the callback
 */
void w5500_httpServer_register_watchdog_callback(void (*callback)(void));

/**
 * @brief Check if the HTTP server is running
 * @return true if running, false otherwise
 */
bool w5500_httpServer_is_running(void);

#endif /* _W5500_HTTPSERVER_H_ */
