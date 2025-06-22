/**
 * @file hello_world.h
 * @brief UDP Hello World utility functions
 */

#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H

#include <stdint.h>

/**
 * @brief Send UDP hello world message using centralized configuration
 * @return Number of bytes sent or negative error code
 */
int32_t hello_world_send_udp(void);

/**
 * @brief Send TCP hello world message to specific target
 * @param dest_ip Destination IP address (4 bytes)
 * @param dest_port Destination port
 * @return Number of bytes sent or negative error code
 */
int32_t hello_world_send_tcp(const uint8_t* dest_ip, uint16_t dest_port);

#endif // HELLO_WORLD_H