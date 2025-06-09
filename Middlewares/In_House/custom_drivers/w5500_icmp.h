/**
 * @file w5500_icmp.h
 * @brief ICMP ping functionality for W5500 Ethernet controller
 * 
 * @details Provides ICMP echo request and response functions for network
 *          connectivity testing using the ping protocol. Uses one UDP socket
 *          to send and receive ICMP packets.
 * 
 * @note    Uses the socket defined by IP_CONFIG_ICMP_SOCKET (default: 1).
 */

#ifndef _W5500_ICMP_H_
#define _W5500_ICMP_H_

#include <stdint.h>
#include <stdbool.h>
#include "ip_config.h"

/* ICMP message types */
#define ICMP_ECHO_REPLY   0
#define ICMP_ECHO_REQUEST 8

/*============================================================================*/
/** @section ICMP/PING SERVICE 
 *  @brief ICMP echo functionality for connectivity testing
 *  @details Handles ICMP echo requests/replies for ping operations.
 *           Socket defined by IP_CONFIG_ICMP_SOCKET.
 *============================================================================*/

/**
 * @brief ICMP error/status codes
 */
typedef enum {
    ICMP_OK = 0,               /**< Successful operation */
    ICMP_ERROR_TIMEOUT = -1,    /**< Ping request timed out */
    ICMP_ERROR_INVALID = -2,    /**< Invalid parameters */
    ICMP_ERROR_SOCKET = -3,     /**< Socket error */
    ICMP_ERROR_SEND = -4        /**< Error sending packet */
} icmp_result_t;

/**
 * @brief Initialize ICMP ping functionality
 * @return true if successful, false otherwise
 */
bool w5500_icmp_init(void);

/**
 * @brief Send a ping to target IP and wait for reply
 * @param target_ip Target IP address (4 bytes)
 * @param timeout_ms Timeout in milliseconds
 * @return Round-trip time in milliseconds if successful, negative value on error
 */
int32_t w5500_icmp_ping(const uint8_t target_ip[4], uint32_t timeout_ms);

/**
 * @brief Process ICMP echo request and send reply (ping responder)
 * @param packet Pointer to the received packet
 * @param len Length of the packet
 * @param src_ip Source IP of the sender
 * @param socket Socket to use for reply
 * @return true if successfully processed and replied, false otherwise
 */
bool w5500_icmp_process(uint8_t *packet, int len, uint8_t *src_ip, int socket);

/**
 * @brief Get ping statistics
 * @param sent Pointer to store packets sent count
 * @param received Pointer to store packets received count
 * @param avg_time Pointer to store average round-trip time
 * @param min_time Pointer to store minimum round-trip time
 * @param max_time Pointer to store maximum round-trip time
 */
void w5500_icmp_get_stats(uint32_t *sent, uint32_t *received, uint32_t *avg_time, 
                    uint32_t *min_time, uint32_t *max_time);

/**
 * @brief Reset ping statistics
 */
void w5500_icmp_reset_stats(void);

/**
 * @brief Set the socket to use for ICMP operations
 * @param socket Socket number to use
 * @return true if successful, false otherwise
 */
bool w5500_icmp_set_socket(uint8_t socket);

#endif /* _W5500_ICMP_H_ */