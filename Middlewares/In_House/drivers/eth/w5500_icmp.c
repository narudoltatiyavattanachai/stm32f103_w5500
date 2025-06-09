/**
 * @file w5500_icmp.c
 * @brief ICMP ping functionality for W5500 Ethernet controller
 * 
 * @details Implementation for ICMP echo request and response functions for
 *          network connectivity testing using the ping protocol.
 */

#include "w5500_icmp.h"
#include "w5500_socket.h"
#include "main.h"
#include <string.h>

/* Enable debug prints if defined */
#ifdef W5500_DEBUG
#define ICMP_DEBUG(...) printf(__VA_ARGS__)
#else
#define ICMP_DEBUG(...) /* Not defined */
#endif

/* ICMP header structure (for direct structure access) */
typedef struct {
    uint8_t  type;          /* ICMP message type */
    uint8_t  code;          /* ICMP message code */
    uint16_t checksum;      /* Checksum of ICMP header and data */
    uint16_t identifier;    /* Used to identify request/reply pairs */
    uint16_t sequence;      /* Sequence number */
    /* Data follows the header */
} icmp_header_t;

/* Ping statistics structure */
typedef struct {
    uint32_t sent;          /* Number of ping packets sent */
    uint32_t received;      /* Number of ping replies received */
    uint32_t avg_time_ms;   /* Average round-trip time in ms */
    uint32_t min_time_ms;   /* Minimum round-trip time in ms */
    uint32_t max_time_ms;   /* Maximum round-trip time in ms */
} ping_stats_t;

/* Configuration constants if not defined in eth_config.h */
#ifndef ETH_CONFIG_ICMP_SOCKET
#define ETH_CONFIG_ICMP_SOCKET     1  /**< Default socket for ICMP */
#endif

#ifndef ETH_CONFIG_PING_BUF_SIZE
#define ETH_CONFIG_PING_BUF_SIZE   64   /**< Buffer size for ICMP packets */
#endif

/* Private variables ---------------------------------------------------------*/
static uint8_t icmp_socket = ETH_CONFIG_ICMP_SOCKET;  /* Socket for ICMP operations */
static uint8_t ping_buffer[ETH_CONFIG_PING_BUF_SIZE]; /* Buffer for ping packets */
static ping_stats_t ping_stats = {0};        /* Ping statistics */
static bool icmp_initialized = false;        /* Initialization flag */

/**
 * @brief Calculate ICMP checksum using standard one's complement sum
 * 
 * @param data Pointer to the data to calculate checksum for
 * @param len Length of the data in bytes
 * @return The calculated 16-bit checksum
 */
static uint16_t icmp_checksum(const uint8_t *data, uint16_t len) {
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
 * @brief Initialize ICMP ping functionality
 * @return true if successful, false otherwise
 */
bool w5500_icmp_init(void)
{
    int8_t ret;
    
    ICMP_DEBUG("[ICMP] Initializing on socket %d...\r\n", icmp_socket);
    
    /* Make sure existing socket is closed first */
    w5500_close(icmp_socket);
    
    /* Create a UDP socket for ICMP */
    ret = w5500_socket(icmp_socket, Sn_MR_UDP, 0, 0);
    if (ret != icmp_socket) {
        ICMP_DEBUG("[ICMP] Socket creation failed (ret=%d)\r\n", ret);
        return false;
    }
    
    /* Initialize ping statistics */
    ping_stats.sent = 0;
    ping_stats.received = 0;
    ping_stats.avg_time_ms = 0;
    ping_stats.min_time_ms = 0xFFFFFFFF;
    ping_stats.max_time_ms = 0;
    
    icmp_initialized = true;
    ICMP_DEBUG("[ICMP] Initialized successfully\r\n");
    return true;
}

/**
 * @brief Process ICMP echo request and send reply (ping responder)
 * @param packet Pointer to the received packet
 * @param len Length of the packet
 * @param src_ip Source IP of the sender
 * @param socket Socket to use for reply
 * @return true if successfully processed and replied, false otherwise
 */
bool w5500_icmp_process(uint8_t *packet, int len, uint8_t *src_ip, int socket)
{
    icmp_header_t *icmp_header;
    uint16_t icmp_len;
    uint16_t checksum;
    
    /* Validate parameters */
    if (packet == NULL || len <= 0 || src_ip == NULL) {
        ICMP_DEBUG("[ICMP] Invalid parameters\r\n");
        return false;
    }
    
    /* Check if this is an ICMP echo request */
    icmp_header = (icmp_header_t *)(packet);
    
    if (icmp_header->type != ICMP_ECHO_REQUEST) {
        ICMP_DEBUG("[ICMP] Not an echo request (type=%d)\r\n", icmp_header->type);
        return false;
    }
    
    /* Convert to echo reply */
    icmp_header->type = ICMP_ECHO_REPLY;
    
    /* Calculate ICMP length (full packet size) */
    icmp_len = len;
    
    /* Reset the checksum */
    icmp_header->checksum = 0;
    
    /* Calculate and set the new checksum */
    checksum = icmp_checksum((const uint8_t *)icmp_header, icmp_len);
    icmp_header->checksum = checksum;
    
    /* Send the reply */
    ICMP_DEBUG("[ICMP] Sending echo reply (%d bytes) to %d.%d.%d.%d\r\n", 
               icmp_len, src_ip[0], src_ip[1], src_ip[2], src_ip[3]);
               
    if (w5500_sendto(socket, packet, icmp_len, src_ip, 0) <= 0) {
        ICMP_DEBUG("[ICMP] Failed to send echo reply\r\n");
        return false;
    }
    
    return true;
}

/**
 * @brief Send a ping to target IP and wait for reply
 * @param target_ip Target IP address (4 bytes)
 * @param timeout_ms Timeout in milliseconds
 * @return Round-trip time in milliseconds if successful, negative value on error
 */
int32_t w5500_icmp_ping(const uint8_t target_ip[4], uint32_t timeout_ms)
{
    icmp_header_t *icmp_header;
    uint8_t i;
    uint8_t reply_ip[4];
    uint16_t recv_port = 0;
    uint32_t start_time, elapsed;
    int32_t recv_len;
    static uint16_t sequence = 0;
    static uint16_t identifier = 0;
    
    /* Initialize identifier if not done already */
    if (identifier == 0) {
        identifier = (uint16_t)HAL_GetTick(); /* Use tick value as identifier base */
    }
    
    /* Make sure ICMP is initialized */
    if (!icmp_initialized && !w5500_icmp_init()) {
        ICMP_DEBUG("[ICMP] Failed to initialize\r\n");
        return ICMP_ERROR_SOCKET;
    }
    
    /* Validate parameters */
    if (!target_ip) {
        ICMP_DEBUG("[ICMP] Invalid target IP\r\n");
        return ICMP_ERROR_INVALID;
    }
    
    /* Initialize ICMP header */
    memset(icmp_buffer, 0, ICMP_MAX_PACKET_SIZE);
    icmp_header = (icmp_header_t *)icmp_buffer;
    icmp_header->type = ICMP_ECHO_REQUEST;
    icmp_header->code = 0;
    icmp_header->identifier = identifier;
    icmp_header->sequence = ++sequence;
    
    /* Fill data portion with incrementing pattern for verification */
    for (i = 0; i < 32; i++) {  /* Use 32 bytes of data */
        icmp_buffer[sizeof(icmp_header_t) + i] = i & 0xFF;
    }
    
    /* Calculate and set checksum */
    icmp_header->checksum = 0;
    icmp_header->checksum = icmp_checksum(icmp_buffer, sizeof(icmp_header_t) + 32);
    
    /* Record start time */
    start_time = HAL_GetTick();
    
    /* Update statistics */
    ping_stats.sent++;
    
    /* Send ICMP echo request packet */
    ICMP_DEBUG("[ICMP] Sending echo request (%d bytes) to %d.%d.%d.%d\r\n", 
                 sizeof(icmp_header_t) + 32, 
                 target_ip[0], target_ip[1], target_ip[2], target_ip[3]);
                 
    if (w5500_sendto(icmp_socket, icmp_buffer, sizeof(icmp_header_t) + 32, 
                    (uint8_t*)target_ip, 0) <= 0) {
        ICMP_DEBUG("[ICMP] Send failed\r\n");
        return ICMP_ERROR_SEND;
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
                icmp_header->identifier == identifier && 
                icmp_header->sequence == sequence) {
                /* Calculate round-trip time */
                elapsed = HAL_GetTick() - start_time;
                
                /* Update statistics */
                ping_stats.received++;
                
                /* Update average time */
                ping_stats.avg_time_ms = (ping_stats.received > 1) ? 
                                     (ping_stats.avg_time_ms * (ping_stats.received - 1) + elapsed) / ping_stats.received : 
                                     elapsed;
                
                if (elapsed < ping_stats.min_time_ms) {
                    ping_stats.min_time_ms = elapsed;
                }
                
                if (elapsed > ping_stats.max_time_ms) {
                    ping_stats.max_time_ms = elapsed;
                }
                
                ICMP_DEBUG("[ICMP] Received reply from %d.%d.%d.%d in %lu ms\r\n",
                                  reply_ip[0], reply_ip[1], reply_ip[2], reply_ip[3], elapsed);
                return (int32_t)elapsed;
            } else {
                ICMP_DEBUG("[ICMP] Received non-matching packet (type=%d, id=%d, seq=%d)\r\n",
                                  icmp_header->type, icmp_header->identifier, icmp_header->sequence);
            }
        }
        
        /* Small delay to prevent CPU overload */
#ifdef USE_CMSIS_OS
        osDelay(1);  /* RTOS-friendly delay */
#else
        HAL_Delay(1); /* Non-RTOS delay */
#endif
    }
    
    /* Timeout occurred */
    ICMP_DEBUG("[ICMP] Ping timeout\r\n");
    return ICMP_ERROR_TIMEOUT;
}

/**
 * @brief Get ping statistics
 * @param sent Pointer to store packets sent count
 * @param received Pointer to store packets received count
 * @param avg_time Pointer to store average round-trip time
 * @param min_time Pointer to store minimum round-trip time
 * @param max_time Pointer to store maximum round-trip time
 */
void w5500_icmp_get_stats(uint32_t *sent, uint32_t *received, uint32_t *avg_time, 
                   uint32_t *min_time, uint32_t *max_time)
{
    if (sent != NULL) {
        *sent = ping_stats.sent;
    }
    
    if (received != NULL) {
        *received = ping_stats.received;
    }
    
    if (avg_time != NULL) {
        *avg_time = ping_stats.avg_time_ms;
    }
    
    if (min_time != NULL) {
        *min_time = (ping_stats.received > 0) ? ping_stats.min_time_ms : 0;
    }
    
    if (max_time != NULL) {
        *max_time = ping_stats.max_time_ms;
    }
}

/**
 * @brief Reset ping statistics
 */
void w5500_icmp_reset_stats(void)
{
    ping_stats.sent = 0;
    ping_stats.received = 0;
    ping_stats.avg_time_ms = 0;
    ping_stats.min_time_ms = 0xFFFFFFFF;
    ping_stats.max_time_ms = 0;
        /* Keep identifier and sequence numbers unchanged */
    ICMP_DEBUG("[ICMP] Statistics reset\r\n");
}

/**
 * @brief Set the socket to use for ICMP operations
 * @param socket Socket number to use
 * @return true if successful, false otherwise
 */
bool w5500_icmp_set_socket(uint8_t socket)
{
    /* Cannot change socket if already initialized */
    if (icmp_initialized) {
        /* Close current socket first */
        w5500_close(icmp_socket);
        icmp_initialized = false;
    }
    
    icmp_socket = socket;
    ICMP_DEBUG("[ICMP] Socket set to %d\r\n", icmp_socket);
    
    /* Re-initialize with the new socket */
    return w5500_icmp_init();
}