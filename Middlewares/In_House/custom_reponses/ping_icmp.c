/**
 * @file ping_icmp.c
 * @brief Implementation of ICMP ping request handling for STM32F103 with W5500
 */

// Import the header file containing ICMP-related definitions
#include "ping_icmp.h"
// Import socket library for network communication functions
#include "socket.h"
// Import standard string manipulation functions
#include <string.h>

/**
 * @brief Calculate ICMP checksum using standard one's complement sum
 * @param data Pointer to the data to calculate checksum for
 * @param len Length of the data in bytes
 * @return The calculated 16-bit checksum
 */
uint16_t icmp_checksum(const uint8_t *data, uint16_t len) {
    // Declare all variables 
    uint32_t sum = 0;
    uint16_t i = 0;
    
    // Process data in 2-byte chunks (network byte order - big endian)
    for (i = 0; i < len - 1; i += 2) {
        // Combine two bytes into a 16-bit value and add to sum
        // First byte shifted left by 8 bits, OR'd with second byte
        sum += (data[i] << 8) | data[i + 1];
    }
    
    // Handle odd-length data by padding last byte with zero
    if (len & 1) {
        // If length is odd, handle the last byte separately
        sum += data[len - 1] << 8;
    }
    
    // Add back any carry bits until no carries remain
    while (sum >> 16) {
        // Add the carry (high 16 bits) back to the low 16 bits
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    // Return one's complement (bitwise NOT) of the sum as final checksum
    return (uint16_t)(~sum);
}

/**
 * @brief Process an ICMP echo request (ping) and send appropriate reply
 * @param packet Pointer to the received IP packet
 * @param len Total length of the packet in bytes
 * @param src_ip Source IP address to send the reply to
 * @param socket_id Socket ID to use for sending the reply
 * @return true if successfully processed and replied, false otherwise
 */
bool icmp_response(uint8_t *packet, int len, uint8_t *src_ip, int socket_id) {
    // Constants for packet structure
    const int IP_HEADER_SIZE = 20;
    const int MIN_ICMP_PACKET_SIZE = 8;
    const int MIN_PACKET_SIZE = IP_HEADER_SIZE + MIN_ICMP_PACKET_SIZE;
    
    // Declare all variables
    uint8_t protocol;
    uint8_t *icmp_header;
    uint16_t icmp_len;
    uint16_t new_chksum;
    int sent_bytes;
    
    // Verify packet is long enough to contain IP header + minimum ICMP header
    if (packet == NULL || src_ip == NULL || len < MIN_PACKET_SIZE) {
        return false; // Invalid input parameters or insufficient data
    }

    // Extract protocol field from IP header (offset 9)
    // IP header starts at offset 0
    protocol = packet[9];
    
    // Verify this is an ICMP packet, otherwise reject it
    if (protocol != IP_PROTO_ICMP) {
        return false;
    }

    // Point to the start of ICMP header (after 20-byte IP header)
    icmp_header = &packet[IP_HEADER_SIZE];
    
    // Check if this is an echo request (ICMP type 8), otherwise reject it
    if (icmp_header[0] != ICMP_ECHO_REQUEST) {
        return false;
    }

    // Modify the packet to make it an echo reply
    // Change ICMP type from Echo Request (8) to Echo Reply (0)
    icmp_header[0] = ICMP_ECHO_REPLY;
    
    // Clear the checksum field (bytes 2-3) before recalculating
    icmp_header[2] = 0;
    icmp_header[3] = 0;

    // Calculate length of ICMP portion (total packet length minus IP header)
    icmp_len = len - IP_HEADER_SIZE;
    
    // Calculate new checksum for the modified ICMP packet
    new_chksum = icmp_checksum(icmp_header, icmp_len);
    
    // Store high byte of checksum in ICMP header byte 2
    icmp_header[2] = (new_chksum >> 8) & 0xFF;
    
    // Store low byte of checksum in ICMP header byte 3
    icmp_header[3] = new_chksum & 0xFF;

    // Send the echo reply packet back to the source IP using the same socket
    sent_bytes = sendto(socket_id, packet, len, src_ip, 0);
    
    // Verify the packet was sent successfully
    return (sent_bytes == len);
}
