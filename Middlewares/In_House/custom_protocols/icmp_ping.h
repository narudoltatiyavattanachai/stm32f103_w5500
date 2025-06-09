#ifndef __ICMP_PING_H__
#define __ICMP_PING_H__

#include <stdint.h>
#include <stdbool.h>
#include "w5500_socket.h"   /* For socket mode definitions */

/* Debug printing support (uncomment to enable debugging) */
/* #define ICMP_DEBUG_ENABLED */

/**
 * @brief Process an ICMP echo request (ping) and send appropriate reply
 * 
 * @param packet Pointer to the received IP packet
 * @param len Total length of the packet in bytes
 * @param src_ip Source IP address to send the reply to
 * @param socket_id Socket ID to use for sending the reply
 * @return true if successfully processed and replied, false otherwise
 */
bool icmp_response(uint8_t *packet, int len, uint8_t *src_ip, int socket_id);

/**
 * @brief Calculate ICMP checksum using standard one's complement sum
 * 
 * @param data Pointer to the data to calculate checksum for
 * @param len Length of the data in bytes
 * @return The calculated 16-bit checksum
 */
uint16_t icmp_checksum(const uint8_t *data, uint16_t len);

/**
 * @brief Initialize ICMP ping functionality
 * 
 * @param socket_num Socket number to use for ICMP operations
 * @return true if successful, false otherwise
 */
bool icmp_ping_init(uint8_t socket_num);

/**
 * @brief Send an ICMP echo request (ping) to a target IP
 * 
 * @param target_ip Target IP address (4 bytes)
 * @param data_size Size of data payload (0-512 bytes)
 * @param ttl Time-to-live value (0 = use default)
 * @param timeout_ms Timeout in milliseconds
 * @return Round-trip time in milliseconds if successful, negative value on error
 */
int32_t icmp_ping_send(const uint8_t target_ip[4], uint16_t data_size, uint8_t ttl, uint32_t timeout_ms);

/**
 * @brief Get ping statistics
 * 
 * @param sent Pointer to store packets sent count
 * @param received Pointer to store packets received count
 * @param avg_time Pointer to store average round-trip time
 * @param min_time Pointer to store minimum round-trip time
 * @param max_time Pointer to store maximum round-trip time
 */
void icmp_get_stats(uint32_t *sent, uint32_t *received, uint32_t *avg_time, 
                   uint32_t *min_time, uint32_t *max_time);

/**
 * @brief Reset ping statistics
 */
void icmp_reset_stats(void);

#endif /* __ICMP_PING_H__ */
