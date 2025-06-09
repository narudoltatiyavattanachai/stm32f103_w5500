/**
 * @file eth_config.h
 * @brief Centralized network configuration for W5500 Ethernet controller
 * 
 * @details This file defines all network parameters, socket assignments, and 
 *          protocol-specific configurations used by all the custom driver modules.
 *          It serves as the central reference point for all W5500 socket-based 
 *          implementations, including DHCP, ICMP, TFTP, micro-ROS, HTTP Server,
 *          and OTA update.
 *
 * @note    MODIFICATION GUIDE:
 *          - Keep socket assignment table updated when adding new functionality
 *          - Each socket (0-7) must be assigned to only one service
 *          - Maintain consistent naming convention for all configuration macros
 *          - Group related parameters in clearly marked sections
 */

#ifndef _ETH_CONFIG_H_
#define _ETH_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/*                          SOCKET ASSIGNMENT TABLE                            */
/*============================================================================*/
/* Socket |  Module File       |  Service               |  Purpose                   */
/* ------ | ------------------ | ---------------------- | -------------------------- */
/* 0      |  w5500_dhcp.c      |  DHCP Client           |  IP Address Assignment     */
/* 1      |  w5500_icmp.c      |  ICMP/Ping             |  Network Diagnostics      */
/* 2      |  Reserved          |  Reserved              |  Reserved for Future Use  */
/* 3      |  w5500_tftp.c      |  TFTP Client           |  File Transfer            */
/* 4      |  w5500_uros.c      |  micro-ROS Transport   |  ROS Communication        */
/* 5      |  w5500_httpServer.c|  Web Server (HTTP)     |  Configuration Interface  */
/* 6      |  Reserved          |  Reserved              |  Reserved for Future Use  */
/* 7      |  w5500_ota.c       |  OTA Firmware Update   |  Firmware Updates         */

/*============================================================================*/
/*                         SOCKET NUMBER DEFINITIONS                           */
/*============================================================================*/
/* These definitions are used by all socket-related modules                    */
/* IMPORTANT: Do not change these values without updating all related modules  */
#define ETH_CONFIG_DHCP_SOCKET         0   /* Socket 0: DHCP Client */
#define ETH_CONFIG_ICMP_SOCKET         1   /* Socket 1: ICMP/Ping */
#define ETH_CONFIG_RESERVED_SOCKET_1   2   /* Socket 2: Reserved for future use */
#define ETH_CONFIG_TFTP_SOCKET         3   /* Socket 3: TFTP Client */
#define ETH_CONFIG_UROS_SOCKET         4   /* Socket 4: micro-ROS Transport */
#define ETH_CONFIG_HTTP_SOCKET         5   /* Socket 5: Web Server (HTTP) */
#define ETH_CONFIG_RESERVED_SOCKET_2   6   /* Socket 6: Reserved for future use */
#define ETH_CONFIG_OTA_SOCKET          7   /* Socket 7: OTA Firmware Update */

/*============================================================================*/
/*                           DEVICE IDENTIFICATION                            */
/*============================================================================*/
/* MAC Address - Must be unique on your local network */
#define ETH_CONFIG_MAC            {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03}

/* Device Information - Used for device discovery and identification */
#define ETH_CONFIG_HOSTNAME       "uros_w5500" /* Device hostname */
#define ETH_CONFIG_DEVICE_TYPE    "uros_edge"       /* Hardware model */
#define ETH_CONFIG_FW_VERSION     "1.0.0"           /* Firmware version */

/*============================================================================*/
/*                            IP CONFIGURATION                                */
/*============================================================================*/
/* IP Acquisition Mode */
#define ETH_CONFIG_USE_DHCP       1     /* 1 = Use DHCP, 0 = Use static IP */

/* Static IP Configuration (used when DHCP is disabled) */
#define ETH_CONFIG_IP             {192, 168, 1, 200}  /* Static IP address */
#define ETH_CONFIG_SUBNET         {255, 255, 255, 0}  /* Subnet mask */
#define ETH_CONFIG_GATEWAY        {192, 168, 1, 1}    /* Default gateway */
#define ETH_CONFIG_DNS            {8, 8, 8, 8}        /* DNS server IP */

/*============================================================================*/
/*                      COMMON NETWORK ERROR CODES                           */
/*============================================================================*/
typedef enum {
    NET_OK               =  0,  /**< Operation successful */
    NET_ERROR            = -1,  /**< General error */
    NET_TIMEOUT          = -2,  /**< Operation timed out */
    NET_BUFFER_ERROR     = -3,  /**< Buffer allocation error */
    NET_SOCKET_ERROR     = -4,  /**< Socket error */
    NET_INVALID_PARAM    = -5,  /**< Invalid parameter */
    NET_CONNECTION_ERROR = -6,  /**< Connection error */
    NET_BUSY             = -7,  /**< Resource busy or in use */
    NET_AUTH_ERROR       = -8,  /**< Authentication error */
    NET_DNS_ERROR        = -9,  /**< DNS resolution error */
    NET_NOT_SUPPORTED    = -10, /**< Feature not supported */
} net_error_t;

/*============================================================================*/
/*                          COMMON TIMEOUT VALUES                            */
/*============================================================================*/
#define ETH_CONFIG_DEFAULT_TIMEOUT    5000   /* Default timeout in milliseconds */
#define ETH_CONFIG_SHORT_TIMEOUT     5000   /* Short timeout in milliseconds */
#define ETH_CONFIG_MEDIUM_TIMEOUT    10000  /* Medium timeout in milliseconds */
#define ETH_CONFIG_LONG_TIMEOUT      30000  /* Long timeout in milliseconds */

/*============================================================================*/
/*                         PROTOCOL CONFIGURATIONS                            */
/*============================================================================*/
/* DHCP Client Configuration */
#define ETH_CONFIG_DHCP_SOCKET    0     /* Socket number for DHCP */
#define ETH_CONFIG_DHCP_RETRY     5     /* Retries if DHCP fails */
#define ETH_CONFIG_DHCP_TIMEOUT   10000 /* Timeout in milliseconds */
#define ETH_CONFIG_DHCP_BUF_SIZE  1024  /* Buffer size for DHCP messages */

/* ICMP/Ping Configuration */
#define ETH_CONFIG_PING_SOCKET    1     /* Socket number for ICMP ping */

/* DNS Client Configuration */
#define ETH_CONFIG_DNS_SOCKET     2     /* Socket number for DNS */
#define ETH_CONFIG_DNS_TIMEOUT    5000  /* DNS query timeout in ms */
#define ETH_CONFIG_DNS_BUF_SIZE   1024  /* Buffer size for DNS messages */

/* TCP/UDP General Parameters */
#define ETH_CONFIG_DEFAULT_PORT   5000  /* Default local port for TCP/UDP */
#define ETH_CONFIG_MTU            1460  /* Maximum Transmission Unit size */
#define ETH_CONFIG_TCP_TIMEOUT    10000 /* TCP connection timeout in ms */

/*============================================================================*/
/*                        COMMON NETWORK DATA STRUCTURES                       */
/*============================================================================*/

/**
 * @brief IP address structure
 */
typedef struct {
    uint8_t addr[4];  /**< IPv4 address in network byte order (4 bytes) */
} ip_addr_t;

/**
 * @brief IP address assignment status values
 */
typedef enum {
    IP_STATUS_NONE = 0,     /**< No IP assigned */
    IP_STATUS_ASSIGNED,      /**< IP assigned successfully */
    IP_STATUS_CHANGED,       /**< IP has changed */
    IP_STATUS_CONFLICT,      /**< IP conflict detected */
} ip_status_t;

/**
 * @brief Network interface configuration structure
 */
typedef struct {
    uint8_t mac[6];        /**< MAC address (6 bytes) */
    uint8_t ip[4];         /**< IP address (4 bytes) */
    uint8_t subnet[4];     /**< Subnet mask (4 bytes) */
    uint8_t gateway[4];    /**< Gateway address (4 bytes) */
    uint8_t dns[4];        /**< DNS server address (4 bytes) */
    bool use_dhcp;         /**< Whether to use DHCP for IP assignment */
} network_config_t;

/**
 * @brief Ping statistics structure
 */
typedef struct {
    uint32_t sent;         /**< Number of ping packets sent */
    uint32_t received;     /**< Number of ping packets received */
    uint32_t lost;         /**< Number of ping packets lost */
    uint32_t avg_time_ms;  /**< Average round-trip time in milliseconds */
    uint32_t min_time_ms;  /**< Minimum round-trip time in milliseconds */
    uint32_t max_time_ms;  /**< Maximum round-trip time in milliseconds */
} ping_stats_t;

/**
 * @brief DNS query result structure
 */
typedef struct {
    uint8_t ip[4];         /**< Resolved IP address (4 bytes) */
    bool resolved;         /**< Whether the hostname was resolved successfully */
    uint32_t time_ms;      /**< Time taken to resolve in milliseconds */
} dns_result_t;

/* End of data types and structures */

/*============================================================================*/
/*                        DHCP CLIENT CONFIGURATION (Socket 0)                */
/*============================================================================*/
/* Configuration parameters for w5500_dhcp.c module */
#define ETH_CONFIG_DHCP_RETRY     5     /* Number of DHCP discovery retries */
#define ETH_CONFIG_DHCP_BUF_SIZE  512   /* Buffer size for DHCP messages (bytes) */
#define ETH_CONFIG_DHCP_TIMEOUT   10000 /* DHCP discovery timeout (ms) */

/*============================================================================*/
/*                        ICMP/PING CONFIGURATION (Socket 1)                  */
/*============================================================================*/
/* Configuration parameters for w5500_icmp.c module */
#define ETH_CONFIG_PING_TIMEOUT   5000  /* Ping response timeout (ms) */
#define ETH_CONFIG_PING_BUF_SIZE  64    /* Buffer size for ICMP packets (bytes) */
#define ETH_CONFIG_PING_ID        0x1234 /* ICMP echo identifier */

/*============================================================================*/
/*                        RESERVED SOCKET 2 CONFIGURATION                     */
/*============================================================================*/
/* Socket 2 is reserved for future use */
/* No configuration parameters defined for this socket */

/*============================================================================*/
/*                        TFTP CLIENT CONFIGURATION (Socket 3)                 */
/*============================================================================*/
/* Configuration parameters for w5500_tftp.c module */
#define ETH_CONFIG_TFTP_PORT       69    /* Standard TFTP server port */
#define ETH_CONFIG_TFTP_BUF_SIZE   1024  /* Buffer size for TFTP transfers (bytes) */
#define ETH_CONFIG_TFTP_TIMEOUT    5000  /* TFTP operation timeout (ms) */
#define ETH_CONFIG_TFTP_RETRIES    5     /* Number of retries for TFTP operations */

/*============================================================================*/
/*                        MICRO-ROS TRANSPORT LAYER (Socket 4)                */
/*============================================================================*/
/* Configuration parameters for w5500_uros.c module */
#define ETH_CONFIG_UROS_LOCAL_PORT    8888  /* Local UDP port for micro-ROS communication */
#define ETH_CONFIG_UROS_AGENT_PORT    8888  /* Remote agent UDP port */
#define ETH_CONFIG_UROS_BUF_SIZE      1024  /* Socket buffer size (bytes) */
#define ETH_CONFIG_UROS_DISC_PORT     8888  /* UDP port for agent discovery */
#define ETH_CONFIG_UROS_DISC_TIMEOUT  1000  /* Discovery timeout (ms) */
#define ETH_CONFIG_UROS_DISC_SIG      "uros-agent" /* Agent signature string */
#define ETH_CONFIG_UROS_DISC_SIG_LEN  10    /* Length of signature */
#define ETH_CONFIG_UROS_MAX_RETRY     10    /* Maximum reconnection attempts */
#define ETH_CONFIG_UROS_INIT_RETRY_MS 100   /* Initial retry delay (ms) */
#define ETH_CONFIG_UROS_MAX_RETRY_MS  5000  /* Maximum retry delay (ms) */
#define ETH_CONFIG_UROS_RETRY_FACTOR  2     /* Exponential backoff multiplier */
#define ETH_CONFIG_UROS_HEARTBEAT_MS  5000  /* Heartbeat check interval (ms) */
#define ETH_CONFIG_UROS_MAX_PKT_SIZE  1024  /* Maximum packet size (bytes) */

/*============================================================================*/
/*                        HTTP SERVER CONFIGURATION (Socket 5)                */
/*============================================================================*/
/* Configuration parameters for w5500_httpServer.c module */
#define ETH_CONFIG_HTTP_PORT         80    /* HTTP server port */
#define ETH_CONFIG_HTTP_BUF_SIZE     1024  /* HTTP server buffer size (bytes) */
#define ETH_CONFIG_HTTP_TIMEOUT      10000 /* HTTP session timeout (ms) */
#define ETH_CONFIG_HTTP_MAX_SOCKETS  2     /* Maximum concurrent HTTP connections */

/*============================================================================*/
/*                        RESERVED SOCKET 6 CONFIGURATION                     */
/*============================================================================*/
/* Socket 6 is reserved for future use */
/* No configuration parameters defined for this socket */

/*============================================================================*/
/*                        OTA UPDATE CONFIGURATION (Socket 7)                 */
/*============================================================================*/
/* Configuration parameters for w5500_ota.c module */
#define ETH_CONFIG_OTA_PORT         5001  /* OTA update service port */
#define ETH_CONFIG_OTA_BUF_SIZE     1024  /* OTA buffer size (bytes) */
#define ETH_CONFIG_OTA_TIMEOUT      30000 /* OTA session timeout (ms) */
#define ETH_CONFIG_OTA_FLASH_SECTOR 0x08   /* Flash sector to start updates */

/*============================================================================*/
/*                         EXTERNAL BUFFER REFERENCES                          */
/*============================================================================*/
/* These references declare buffers that are defined in their respective modules */
/* They allow cross-module awareness of buffer locations without tight coupling */

/* Socket 0: DHCP Client Buffer */
extern uint8_t dhcp_buffer[ETH_CONFIG_DHCP_BUF_SIZE];

/* Socket 1: ICMP/Ping Buffer */
extern uint8_t ping_buffer[ETH_CONFIG_PING_BUF_SIZE];

/* Socket 3: TFTP Client Buffer */
extern uint8_t tftp_buffer[ETH_CONFIG_TFTP_BUF_SIZE];

/* Socket 4: micro-ROS Transport Buffer */
extern uint8_t uros_buffer[ETH_CONFIG_UROS_BUF_SIZE];

/* Socket 5: HTTP Server Buffer */
extern uint8_t http_buffer[ETH_CONFIG_HTTP_BUF_SIZE];

/* Socket 7: OTA Update Buffer */
extern uint8_t ota_buffer[ETH_CONFIG_OTA_BUF_SIZE];

#endif /* _ETH_CONFIG_H_ */

/* End of eth_config.h */