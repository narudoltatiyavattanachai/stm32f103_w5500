#ifndef __PING_ICMP_H__
#define __PING_ICMP_H__

#include <stdint.h>
#include <stdbool.h>

#define ICMP_ECHO_REQUEST 0x08
#define ICMP_ECHO_REPLY   0x00
#define IP_PROTO_ICMP     0x01

#define ICMP_BUFFER_SIZE 64

bool icmp_response(uint8_t *packet, int len, uint8_t *src_ip, int socket_id);
uint16_t icmp_checksum(const uint8_t *data, uint16_t len);

#endif // __PING_ICMP_H__
