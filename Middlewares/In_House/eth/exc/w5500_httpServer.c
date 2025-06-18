/**
 * @file w5500_httpServer.c
 * @brief HTTP server functionality for W5500 Ethernet controller
 * 
 * @details Implementation of the modular wrapper around the third-party HTTP server library
 *          for serving web content over Ethernet.
 */

#include "w5500_httpServer.h"
#include "w5500_socket.h"
#include "main.h"
#include <string.h>

/* Include the third-party HTTP server library */
#include "httpServer.h"

/* Enable debug prints if defined */
#ifdef W5500_DEBUG
#define HTTP_DEBUG(...) printf(__VA_ARGS__)
#else
#define HTTP_DEBUG(...) /* Not defined */
#endif

/* Configuration constants if not defined in eth_config.h */
#ifndef ETH_HTTP_BUF_SIZE
#define ETH_HTTP_BUF_SIZE   2048  /**< Default HTTP buffer size */
#endif

#ifndef ETH_HTTP_PORT
#define ETH_HTTP_PORT   80  /**< Default HTTP server port */
#endif

#ifndef ETH_HTTP_MAX_SOCKETS
#define ETH_HTTP_MAX_SOCKETS 4  /**< Maximum concurrent HTTP connections */
#endif

#ifndef ETH_HTTP_TIMEOUT
#define ETH_HTTP_TIMEOUT    10    /**< Default HTTP timeout in seconds */
#endif

/* Private variables ---------------------------------------------------------*/
static bool http_server_initialized = false;     /* Initialization flag */
static uint8_t http_socket_list[_WIZCHIP_SOCK_NUM_]; /* List of sockets for HTTP */
static uint8_t http_max_sockets = ETH_HTTP_MAX_SOCKETS; /* Max sockets for HTTP */
static uint16_t http_server_port = ETH_HTTP_PORT; /* HTTP server port */

/* HTTP buffer allocation */
static uint8_t *http_tx_buf = NULL;
static uint8_t *http_rx_buf = NULL;
static bool using_internal_buffers = false;

/* Content-Type lookup table */
static const char *content_type_table[] = {
    "text/html",           /* HTTP_CONTENT_HTML */
    "text/css",            /* HTTP_CONTENT_CSS */
    "application/javascript", /* HTTP_CONTENT_JS */
    "application/json",    /* HTTP_CONTENT_JSON */
    "image/png",           /* HTTP_CONTENT_PNG */
    "image/jpeg",          /* HTTP_CONTENT_JPG */
    "image/x-icon",        /* HTTP_CONTENT_ICO */
    "text/plain",          /* HTTP_CONTENT_TEXT */
    "application/octet-stream" /* HTTP_CONTENT_BINARY */
};

/**
 * @brief Initialize HTTP server with default settings
 * @return true if successful, false otherwise
 */
bool w5500_httpServer_init(void)
{
    uint8_t i;
    http_server_config_t default_config;
    
    /* Create default configuration */
    default_config.port = ETH_HTTP_PORT;
    default_config.max_sockets = ETH_HTTP_MAX_SOCKETS;
    default_config.timeout = ETH_HTTP_TIMEOUT;
    
    /* Allocate memory for HTTP buffers if not already allocated */
    if (http_tx_buf == NULL && http_rx_buf == NULL) {
        http_tx_buf = (uint8_t*)malloc(ETH_HTTP_BUF_SIZE);
        http_rx_buf = (uint8_t*)malloc(ETH_HTTP_BUF_SIZE);
        
        if (http_tx_buf == NULL || http_rx_buf == NULL) {
            HTTP_DEBUG("[HTTP] Failed to allocate buffers\r\n");
            
            /* Free any allocated memory */
            if (http_tx_buf != NULL) {
                free(http_tx_buf);
                http_tx_buf = NULL;
            }
            
            if (http_rx_buf != NULL) {
                free(http_rx_buf);
                http_rx_buf = NULL;
            }
            
            return false;
        }
        
        using_internal_buffers = true;
    }
    
    default_config.tx_buffer = http_tx_buf;
    default_config.rx_buffer = http_rx_buf;
    
    /* Initialize with default configuration */
    return w5500_httpServer_init_with_config(&default_config);
}

/**
 * @brief Initialize HTTP server with custom configuration
 * @param config Pointer to configuration structure
 * @return true if successful, false otherwise
 */
bool w5500_httpServer_init_with_config(const http_server_config_t *config)
{
    uint8_t i;
    
    /* Parameter validation */
    if (config == NULL || config->tx_buffer == NULL || config->rx_buffer == NULL) {
        HTTP_DEBUG("[HTTP] Invalid configuration\r\n");
        return false;
    }
    
    /* Configure local settings */
    http_server_port = config->port;
    http_max_sockets = (config->max_sockets <= _WIZCHIP_SOCK_NUM_) ? 
                        config->max_sockets : _WIZCHIP_SOCK_NUM_;
    
    /* If not using our internal buffers, use the provided ones */
    if (!using_internal_buffers) {
        http_tx_buf = config->tx_buffer;
        http_rx_buf = config->rx_buffer;
    }
    
    /* Set up socket list - start from socket 0 and use consecutive sockets */
    for (i = 0; i < http_max_sockets; i++) {
        http_socket_list[i] = i;
    }
    
    /* Initialize the HTTP server from third-party library */
    httpServer_init(http_tx_buf, http_rx_buf, http_max_sockets, http_socket_list);
    
    /* Register default callbacks */
    reg_httpServer_cbfunc(NULL, NULL);
    
    HTTP_DEBUG("[HTTP] Server initialized on port %d with %d sockets\r\n", 
             http_server_port, http_max_sockets);
    
    http_server_initialized = true;
    return true;
}

/**
 * @brief Run the HTTP server (process connections) - call in main loop
 * @return HTTP_SERVER_OK if successful, error code otherwise
 */
http_server_result_t w5500_httpServer_run(void)
{
    uint8_t i;
    
    /* Make sure server is initialized */
    if (!http_server_initialized) {
        HTTP_DEBUG("[HTTP] Server not initialized\r\n");
        return HTTP_SERVER_ERROR_INIT;
    }
    
    /* Process each HTTP socket */
    for (i = 0; i < http_max_sockets; i++) {
        httpServer_run(i);
    }
    
    return HTTP_SERVER_OK;
}

/**
 * @brief HTTP server timer handler - call every 1 second
 */
void w5500_httpServer_timer_handler(void)
{
    if (http_server_initialized) {
        httpServer_time_handler();
    }
}

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
    http_content_type_t content_type)
{
    /* Make sure server is initialized */
    if (!http_server_initialized) {
        HTTP_DEBUG("[HTTP] Server not initialized\r\n");
        return false;
    }
    
    /* Parameter validation */
    if (name == NULL || content == NULL || content_len == 0) {
        HTTP_DEBUG("[HTTP] Invalid content parameters\r\n");
        return false;
    }
    
    /* Register content with the HTTP server */
    reg_httpServer_webContent((uint8_t *)name, (uint8_t *)content);
    
    HTTP_DEBUG("[HTTP] Registered web content: %s\r\n", name);
    
    return true;
}

/**
 * @brief Register a callback to be called when MCU reset is requested via web interface
 * @param callback Function pointer to the callback
 */
void w5500_httpServer_register_reset_callback(void (*callback)(void))
{
    if (callback != NULL) {
        reg_httpServer_cbfunc(callback, NULL);
        HTTP_DEBUG("[HTTP] Reset callback registered\r\n");
    }
}

/**
 * @brief Register a callback to be called for watchdog reset
 * @param callback Function pointer to the callback
 */
void w5500_httpServer_register_watchdog_callback(void (*callback)(void))
{
    if (callback != NULL) {
        /* Get the current MCU reset callback */
        extern void (*HTTPServer_ReStart)(void);
        
        /* Register both callbacks */
        reg_httpServer_cbfunc(HTTPServer_ReStart, callback);
        HTTP_DEBUG("[HTTP] Watchdog callback registered\r\n");
    }
}

/**
 * @brief Check if the HTTP server is running
 * @return true if running, false otherwise
 */
bool w5500_httpServer_is_running(void)
{
    return http_server_initialized;
}

/**
 * @brief Clean up HTTP server resources
 * @note Should be called on application shutdown
 */
void w5500_httpServer_close(void)
{
    uint8_t i;
    
    /* Close all HTTP sockets */
    if (http_server_initialized) {
        for (i = 0; i < http_max_sockets; i++) {
            w5500_close(http_socket_list[i]);
        }
    }
    
    /* Free allocated memory if we're using internal buffers */
    if (using_internal_buffers) {
        if (http_tx_buf != NULL) {
            free(http_tx_buf);
            http_tx_buf = NULL;
        }
        
        if (http_rx_buf != NULL) {
            free(http_rx_buf);
            http_rx_buf = NULL;
        }
        
        using_internal_buffers = false;
    }
    
    http_server_initialized = false;
    HTTP_DEBUG("[HTTP] Server closed\r\n");
}
