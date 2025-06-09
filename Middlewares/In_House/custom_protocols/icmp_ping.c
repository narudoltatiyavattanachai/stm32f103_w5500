/**
 * @file icmp_ping.c
 * @brief Implementation of ICMP ping functionality for STM32F103 with W5500
 * 
 * @details This file provides functions for sending and responding to ICMP echo
 *          requests (ping). It uses the W5500 socket API for all network operations,
 *          ensuring proper separation from the hardware layer. The implementation
 *          supports both responding to incoming pings and actively pinging remote
 *          hosts with statistics tracking.
 */

#include "icmp_ping.h"
#include "w5500_socket.h"   /* W5500 socket API */
#include "stm32f1xx_hal.h"  /* For HAL_GetTick() */
#include <string.h>         /* For memcpy, memset */

/* Debug printing support (can be disabled) */
#ifdef ICMP_DEBUG_ENABLED
#include <stdio.h>  /* For printf */
#define ICMP_DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define ICMP_DEBUG_PRINT(fmt, ...) /* No print */
#endif

/* Constants and type definitions ------------------------------------------*/

/* ICMP packet types */
#define ICMP_ECHO_REQUEST 0x08
#define ICMP_ECHO_REPLY   0x00
#define IP_PROTO_ICMP     0x01

/* ICMP header structure (for direct structure access) */
typedef struct {
    uint8_t  type;          /* ICMP message type */
    uint8_t  code;          /* ICMP message code */
    uint16_t checksum;      /* Checksum of ICMP header and data */
    uint16_t identifier;    /* Used to identify request/reply pairs */
    uint16_t sequence;      /* Sequence number */
    /* Data follows the header */
} icmp_header_t;

/* Structure to hold ping statistics */
typedef struct {
    uint32_t packets_sent;       /* Number of packets sent */
    uint32_t packets_received;   /* Number of packets received */
    uint32_t total_time_ms;      /* Total round-trip time in ms */
    uint32_t min_time_ms;        /* Minimum round-trip time in ms */
    uint32_t max_time_ms;        /* Maximum round-trip time in ms */
    uint16_t last_sequence;      /* Last sequence number used */
    uint16_t identifier;         /* Current ping identifier */
} ping_stats_t;

/* Maximum size for ping buffer */
#define ICMP_MAX_PACKET_SIZE 576

/* Private variables ---------------------------------------------------------*/
static uint8_t icmp_socket = 0xFF;           /* Socket for ICMP operations */
static uint8_t icmp_buffer[ICMP_MAX_PACKET_SIZE]; /* Buffer for ping packets */
static ping_stats_t ping_stats = {0};        /* Ping statistics */

/**
 * @brief Calculate ICMP checksum using standard one's complement sum
 * 
 * @param data Pointer to the data to calculate checksum for
 * @param len Length of the data in bytes
 * @return The calculated 16-bit checksum
 */
uint16_t icmp_checksum(const uint8_t *data, uint16_t len) {
    uint32_t sum = 0;
    uint16_t i;
    
    /* Check for null pointer */
    if (data == NULL) {
        return 0;
    }
    
    /* Process data in 2-byte chunks (network byte order) */
    for (i = 0; i < len - 1; i += 2) {
        sum += (data[i] << 8) | data[i + 1];
    }
    
    /* Handle odd-length data by padding last byte with zero */
    if (len & 1) {
        sum += data[len - 1] << 8;
    }
    
    /* Add back any carry bits until no carries remain */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    /* Return one's complement of the sum as final checksum */
    return (uint16_t)(~sum);
}

/**
 * @brief Process an ICMP echo request (ping) and send appropriate reply
 * 
 * @param packet Pointer to the received IP packet
 * @param len Total length of the packet in bytes
 * @param src_ip Source IP address to send the reply to
 * @param socket_id Socket ID to use for sending the reply
 * @return true if successfully processed and replied, false otherwise
 */
bool icmp_response(uint8_t *packet, int len, uint8_t *src_ip, int socket_id) {
    /* Constants for packet structure */
    const int IP_HEADER_SIZE = 20;
    const int MIN_ICMP_PACKET_SIZE = 8;
    const int MIN_PACKET_SIZE = IP_HEADER_SIZE + MIN_ICMP_PACKET_SIZE;
    
    uint8_t protocol;
    uint8_t *icmp_header;
    uint16_t icmp_len;
    uint16_t new_chksum;
    int sent_bytes;
    
    /* Input validation */
    if (packet == NULL || src_ip == NULL || len < MIN_PACKET_SIZE) {
        ICMP_DEBUG_PRINT("ICMP: Invalid packet parameters\r\n");
        return false;
    }

    /* Check if this is an ICMP packet */
    protocol = packet[9];  /* Protocol field is at offset 9 in IP header */
    if (protocol != IP_PROTO_ICMP) {
        ICMP_DEBUG_PRINT("ICMP: Not an ICMP packet (protocol=%d)\r\n", protocol);
        return false;
    }

    /* Get pointer to ICMP header (after IP header) */
    icmp_header = &packet[IP_HEADER_SIZE];
    
    /* Check if this is an echo request */
    if (icmp_header[0] != ICMP_ECHO_REQUEST) {
        ICMP_DEBUG_PRINT("ICMP: Not an echo request (type=%d)\r\n", icmp_header[0]);
        return false;
    }

    ICMP_DEBUG_PRINT("ICMP: Processing echo request from %d.%d.%d.%d\r\n",
                     src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

    /* Convert echo request to echo reply */
    icmp_header[0] = ICMP_ECHO_REPLY;  /* Change type to echo reply */
    
    /* Clear the checksum field before recalculating */
    icmp_header[2] = 0;
    icmp_header[3] = 0;

    /* Calculate length of ICMP portion and new checksum */
    icmp_len = len - IP_HEADER_SIZE;
    new_chksum = icmp_checksum(icmp_header, icmp_len);
    
    /* Insert new checksum into ICMP header */
    icmp_header[2] = (new_chksum >> 8) & 0xFF;  /* High byte */
    icmp_header[3] = new_chksum & 0xFF;         /* Low byte */

    /* Send echo reply back to source */
    sent_bytes = w5500_sendto(socket_id, packet, len, src_ip, 0);
    
    if (sent_bytes == len) {
        ICMP_DEBUG_PRINT("ICMP: Echo reply sent successfully\r\n");
        return true;
    } else {
        ICMP_DEBUG_PRINT("ICMP: Failed to send echo reply (sent %d/%d bytes)\r\n", 
                         sent_bytes, len);
        return false;
    }
}

/**
 * @brief Initialize ICMP ping functionality
 * 
 * @param socket_num Socket number to use for ICMP operations
 * @return true if successful, false otherwise
 */
bool icmp_ping_init(uint8_t socket_num) {
    ICMP_DEBUG_PRINT("ICMP: Initializing ping on socket %d\r\n", socket_num);
    
    /* Close the socket if it was already in use */
    w5500_close(socket_num);
    
    /* Open as IPRAW socket with ICMP protocol */
    if (w5500_socket(socket_num, Sn_MR_IPRAW, 0, IP_PROTO_ICMP) != socket_num) {
        ICMP_DEBUG_PRINT("ICMP: Failed to create socket\r\n");
        return false;
    }
    
    /* Store the socket number for later use */
    icmp_socket = socket_num;
    
    /* Initialize ping statistics */
    memset(&ping_stats, 0, sizeof(ping_stats));
    ping_stats.identifier = 0x1234;  /* Arbitrary identifier */
    ping_stats.min_time_ms = 0xFFFFFFFF;  /* Set to max value initially */
    
    ICMP_DEBUG_PRINT("ICMP: Ping initialized successfully\r\n");
    return true;
}

/**
 * @brief Send an ICMP echo request (ping) to a target IP
 * 
 * @param target_ip Target IP address (4 bytes)
 * @param data_size Size of data payload (0-512 bytes)
 * @param ttl Time-to-live value (0 = use default)
 * @param timeout_ms Timeout in milliseconds
 * @return Round-trip time in milliseconds if successful, negative value on error
 */
int32_t icmp_ping_send(const uint8_t target_ip[4], uint16_t data_size, uint8_t ttl, uint32_t timeout_ms) {
    icmp_header_t *icmp_header;
    uint16_t checksum;
    uint32_t start_time, elapsed;
    int32_t recv_len;
    uint32_t i;
    uint8_t reply_ip[4];
    uint16_t recv_port = 0;
    
    /* Validate parameters */
    if (icmp_socket == 0xFF) {
        ICMP_DEBUG_PRINT("ICMP: Ping socket not initialized\r\n");
        return -1;
    }
    
    if (data_size > (ICMP_MAX_PACKET_SIZE - sizeof(icmp_header_t))) {
        data_size = ICMP_MAX_PACKET_SIZE - sizeof(icmp_header_t);
        ICMP_DEBUG_PRINT("ICMP: Limiting data size to %d bytes\r\n", data_size);
    }
    
    /* Set TTL if specified */
    if (ttl > 0) {
        w5500_setsockopt(icmp_socket, SO_TTL, &ttl);
    }
    
    /* Set up ICMP header */
    memset(icmp_buffer, 0, ICMP_MAX_PACKET_SIZE);
    icmp_header = (icmp_header_t *)icmp_buffer;
    
    icmp_header->type = ICMP_ECHO_REQUEST;
    icmp_header->code = 0;
    icmp_header->identifier = ping_stats.identifier;
    icmp_header->sequence = ++ping_stats.last_sequence;
    
    /* Fill data portion with incrementing pattern */
    for (i = 0; i < data_size; i++) {
        icmp_buffer[sizeof(icmp_header_t) + i] = i & 0xFF;
    }
    
    /* Calculate checksum */
    icmp_header->checksum = 0;
    checksum = icmp_checksum(icmp_buffer, sizeof(icmp_header_t) + data_size);
    icmp_header->checksum = checksum;
    
    /* Record statistics */
    ping_stats.packets_sent++;
    
    /* Record start time */
    start_time = HAL_GetTick();
    
    /* Send the ICMP echo request */
    ICMP_DEBUG_PRINT("ICMP: Sending ping to %d.%d.%d.%d with %d bytes of data\r\n",
                     target_ip[0], target_ip[1], target_ip[2], target_ip[3], data_size);
                     
    if (w5500_sendto(icmp_socket, icmp_buffer, sizeof(icmp_header_t) + data_size, 
                    (uint8_t*)target_ip, 0) <= 0) {
        ICMP_DEBUG_PRINT("ICMP: Send failed\r\n");
        return -2;
    }
    
    /* Wait for reply */
    while ((HAL_GetTick() - start_time) < timeout_ms) {
        /* Check if there's data to read */
        if (w5500_getsockopt(icmp_socket, SO_RECVBUF, (void*)&i) > 0 && i > 0) {
            /* Receive the reply */
            memset(icmp_buffer, 0, ICMP_MAX_PACKET_SIZE);
            recv_len = w5500_recvfrom(icmp_socket, icmp_buffer, ICMP_MAX_PACKET_SIZE,
                                    reply_ip, &recv_port);
                                    
            if (recv_len <= 0) {
                continue;  /* Error or no data available */
            }
            
            /* Process the ICMP reply */
            icmp_header = (icmp_header_t *)(icmp_buffer);
            
            /* Check if this is an ICMP echo reply with our identifier and sequence */
            if (icmp_header->type == ICMP_ECHO_REPLY && 
                icmp_header->identifier == ping_stats.identifier && 
                icmp_header->sequence == ping_stats.last_sequence) {
                /* Calculate round-trip time */
                elapsed = HAL_GetTick() - start_time;
                
                /* Update statistics */
                ping_stats.packets_received++;
                ping_stats.total_time_ms += elapsed;
                
                if (elapsed < ping_stats.min_time_ms) {
                    ping_stats.min_time_ms = elapsed;
                }
                
                if (elapsed > ping_stats.max_time_ms) {
                    ping_stats.max_time_ms = elapsed;
                }
                
                ICMP_DEBUG_PRINT("ICMP: Received reply from %d.%d.%d.%d in %lu ms\r\n",
                                 reply_ip[0], reply_ip[1], reply_ip[2], reply_ip[3], elapsed);
                return (int32_t)elapsed;
            } else {
                ICMP_DEBUG_PRINT("ICMP: Received non-matching packet (type=%d, id=%d, seq=%d)\r\n",
                                 icmp_header->type, icmp_header->identifier, icmp_header->sequence);
            }
        }
        
        /* Small delay to prevent CPU overload */
        HAL_Delay(1);
    }
    
    /* Timeout occurred */
    ICMP_DEBUG_PRINT("ICMP: Ping timeout\r\n");
    return -3;
}

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
                   uint32_t *min_time, uint32_t *max_time) {
    if (sent != NULL) {
        *sent = ping_stats.packets_sent;
    }
    
    if (received != NULL) {
        *received = ping_stats.packets_received;
    }
    
    if (avg_time != NULL) {
        *avg_time = (ping_stats.packets_received > 0) ? 
                    (ping_stats.total_time_ms / ping_stats.packets_received) : 0;
    }
    
    if (min_time != NULL) {
        *min_time = (ping_stats.packets_received > 0) ? ping_stats.min_time_ms : 0;
    }
    
    if (max_time != NULL) {
        *max_time = ping_stats.max_time_ms;
    }
}

/**
 * @brief Reset ping statistics
 */
void icmp_reset_stats(void) {
    ping_stats.packets_sent = 0;
    ping_stats.packets_received = 0;
    ping_stats.total_time_ms = 0;
    ping_stats.min_time_ms = 0xFFFFFFFF;
    ping_stats.max_time_ms = 0;
    /* Keep the identifier and sequence number */
}
