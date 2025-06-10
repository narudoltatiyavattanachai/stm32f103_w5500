// Modified w5500_socket.c content

/**
 * @file     w5500_socket.c
 * @brief    Socket API wrapper implementation for W5500 Ethernet controller
 * @author   Narudol T.
 * @date     2025-06-10
 *
 * @details  This file implements wrapper functions around the third-party socket API
 * (WIZnet ioLibrary_Driver) to isolate application code from direct
 * dependencies on third-party libraries. All socket operations should use
 * these wrappers instead of calling socket.h functions directly.
 *
 * @note     Usage:
 * - Provides a stable, testable, and maintainable socket abstraction.
 * - All configuration is centralized via eth_config.h macros.
 * - Debugging can be enabled with W5500_DEBUG.
 *
 * Maintenance:
 * - Keep all buffer sizes, timeouts, and socket numbers in eth_config.h.
 * - Use section headers and Doxygen-style comments for clarity.
 */
// Must include wizchip_conf.h first to set _WIZCHIP_ macro
#include "../../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/wizchip_conf.h"
#include "../../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/W5500/w5500.h"
#include "../../../Middlewares/Third_Party/ioLibrary_Driver_v3.2.0/Ethernet/socket.h"

// Standard includes
#include <string.h>
#include <stdio.h>
#include "stm32f1xx_hal.h" // For HAL debug output

// Local includes
#include "w5500_spi.h"  // Must come after WIZnet includes
#include "eth_config.h"
#include "w5500_socket.h"

/*============================================================================*/
/* DEBUG PRINT MACRO                                  */
/*============================================================================*/
#ifdef W5500_DEBUG
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) (void)0
#endif

/*============================================================================*/
/* SOCKET INITIALIZATION/DEINITIALIZATION             */
/*============================================================================*/

/**
 * @brief Open a socket and initialize it
 *
 * @param sock_num  Socket number (0-7)
 * @param type      Socket type (TCP or UDP)
 * @param port      Port number to bind to
 * @return int8_t   W5500_SOCK_OK on success, negative error code on failure
 */
int8_t w5500_socket_open(uint8_t sock_num, w5500_sock_type_t type, uint16_t port)
{
    uint8_t protocol;
    uint8_t flag = 0; // Default flags

    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_open: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }

    switch (type)
    {
    case W5500_SOCK_TCP:
        protocol = Sn_MR_TCP;
        break;
    case W5500_SOCK_UDP:
        protocol = Sn_MR_UDP;
        break;
    default:
        DEBUG_PRINT("w5500_socket_open: Unsupported socket type %d\r\n", type);
        return W5500_SOCK_ERROR;
    }

    DEBUG_PRINT("w5500_socket_open: Opening socket %d, type %d, port %d\r\n", sock_num, type, port);
    // The 'socket' function is from the WIZnet ioLibrary_Driver
    int8_t ret = socket(sock_num, protocol, port, flag);

    if (ret != sock_num) // On success, socket() returns the socket number
    {
        DEBUG_PRINT("w5500_socket_open: Failed to open socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }

    return W5500_SOCK_OK;
}

/**
 * @brief Close a socket
 *
 * @param sock_num  Socket number
 * @return int8_t   W5500_SOCK_OK on success, negative error code on failure
 */
int8_t w5500_socket_close(uint8_t sock_num)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_close: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }

    DEBUG_PRINT("w5500_socket_close: Closing socket %d\r\n", sock_num);
    // The 'close' function is from the WIZnet ioLibrary_Driver
    int8_t ret = close(sock_num);

    if (ret != SOCK_OK) // SOCK_OK is usually 0
    {
        DEBUG_PRINT("w5500_socket_close: Failed to close socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }

    return W5500_SOCK_OK;
}

/**
 * @brief Set a socket option
 *
 * @param sock_num      Socket number
 * @param option_type   Type of option to set (e.g., SO_SENDBUF, SO_RCVBUF)
 * @param option_value  Pointer to the value to set
 * @return int8_t       W5500_SOCK_OK on success, negative error code on failure
 */
int8_t w5500_socket_setsockopt(uint8_t sock_num, uint8_t option_type, void *option_value)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_setsockopt: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_socket_setsockopt: Setting option %d on socket %d\r\n", option_type, sock_num);
    // The 'setsockopt' function is from the WIZnet ioLibrary_Driver
    int8_t ret = setsockopt(sock_num, option_type, option_value);
    if (ret != SOCK_OK)
    {
        DEBUG_PRINT("w5500_socket_setsockopt: Failed to set option %d on socket %d, error %d\r\n", option_type, sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    return W5500_SOCK_OK;
}

/**
 * @brief Get a socket option
 *
 * @param sock_num      Socket number
 * @param option_type   Type of option to get
 * @param option_value  Pointer to store the option value
 * @return int8_t       W5500_SOCK_OK on success, negative error code on failure
 */
int8_t w5500_socket_getsockopt(uint8_t sock_num, uint8_t option_type, void *option_value)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_getsockopt: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    int8_t ret = getsockopt(sock_num, option_type, option_value);
    DEBUG_PRINT("w5500_socket_getsockopt: Getting option %d from socket %d, result %d\r\n",
                option_type, sock_num, ret);
    if (ret != SOCK_OK)
    {
        DEBUG_PRINT("w5500_socket_getsockopt: Failed to get option %d from socket %d, error %d\r\n", option_type, sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    return W5500_SOCK_OK;
}

/*============================================================================*/
/* TCP SPECIFIC OPERATIONS                            */
/*============================================================================*/

/**
 * @brief Connect a TCP socket to a remote host
 *
 * @param sock_num  Socket number
 * @param dest_ip   Destination IP address (4 bytes)
 * @param dest_port Destination port
 * @return int8_t   W5500_SOCK_OK on success, negative error code on failure
 */
int8_t w5500_socket_connect(uint8_t sock_num, const uint8_t *dest_ip, uint16_t dest_port)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_connect: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_socket_connect: Connecting socket %d to %d.%d.%d.%d:%d\r\n",
                sock_num, dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3], dest_port);
    // The 'connect' function is from the WIZnet ioLibrary_Driver
    int8_t ret = connect(sock_num, (uint8_t *)dest_ip, dest_port);
    if (ret != SOCK_OK)
    {
        DEBUG_PRINT("w5500_socket_connect: Failed to connect socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    return W5500_SOCK_OK;
}

/**
 * @brief Disconnect a TCP socket (Graceful close for TCP)
 *
 * @param sock_num Socket number
 * @return int8_t W5500_SOCK_OK on success, negative error code on failure
 */
int8_t w5500_disconnect(uint8_t sock_num)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_disconnect: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_disconnect: Disconnecting socket %d\r\n", sock_num);
    // The 'disconnect' function is from the WIZnet ioLibrary_Driver, specifically for TCP
    int8_t ret = disconnect(sock_num);
    if (ret != SOCK_OK)
    {
        DEBUG_PRINT("w5500_disconnect: Failed to disconnect socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    return W5500_SOCK_OK;
}

/**
 * @brief Listen for incoming TCP connections
 *
 * @param sock_num  Socket number
 * @return int8_t   W5500_SOCK_OK on success, negative error code on failure
 */
int8_t w5500_socket_listen(uint8_t sock_num)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_listen: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_socket_listen: Listening on socket %d\r\n", sock_num);
    // The 'listen' function is from the WIZnet ioLibrary_Driver
    int8_t ret = listen(sock_num);
    if (ret != SOCK_OK)
    {
        DEBUG_PRINT("w5500_socket_listen: Failed to listen on socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    return W5500_SOCK_OK;
}

/**
 * @brief Check if a TCP socket is connected
 *
 * @param sock_num  Socket number
 * @return bool     true if connected, false otherwise
 */
bool w5500_socket_is_connected(uint8_t sock_num)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_is_connected: Invalid socket number %d\r\n", sock_num);
        return false;
    }
    // This typically checks the socket status register (Sn_SR)
    // Sn_SR_ESTABLISHED indicates a connected state for TCP.
    // getSn_SR is from wizchip_conf.h or socket.h in WIZnet library.
    uint8_t status = getSn_SR(sock_num);
    DEBUG_PRINT("w5500_socket_is_connected: Socket %d status: 0x%02X\r\n", sock_num, status);
    return (status == SOCK_ESTABLISHED);
}

/*============================================================================*/
/* DATA TRANSFER FUNCTIONS                            */
/*============================================================================*/

/**
 * @brief Send data over a socket
 *
 * @param sock_num  Socket number
 * @param buffer    Pointer to data to send
 * @param len       Length of data to send
 * @return int32_t  Number of bytes sent, negative error code on failure
 */
int32_t w5500_socket_send(uint8_t sock_num, const uint8_t *buffer, uint16_t len)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_send: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    if (buffer == NULL)
    {
        DEBUG_PRINT("w5500_socket_send: Buffer is NULL\r\n");
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_socket_send: Sending %d bytes on socket %d\r\n", len, sock_num);
    // The 'send' function is from the WIZnet ioLibrary_Driver
    int32_t ret = send(sock_num, (uint8_t *)buffer, len);
    if (ret < 0)
    {
        DEBUG_PRINT("w5500_socket_send: Failed to send data on socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    return ret;
}

/**
 * @brief Receive data from a socket
 *
 * @param sock_num  Socket number
 * @param buffer    Buffer to store received data
 * @param maxlen    Maximum length of buffer
 * @return int32_t  Number of bytes received, negative error code on failure
 */
int32_t w5500_socket_recv(uint8_t sock_num, uint8_t *buffer, uint16_t maxlen)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_recv: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    if (buffer == NULL)
    {
        DEBUG_PRINT("w5500_socket_recv: Buffer is NULL\r\n");
        return W5500_SOCK_ERROR;
    }
    // The 'recv' function is from the WIZnet ioLibrary_Driver
    int32_t ret = recv(sock_num, buffer, maxlen);
    if (ret < 0)
    {
        DEBUG_PRINT("w5500_socket_recv: Failed to receive data on socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_socket_recv: Received %d bytes on socket %d\r\n", ret, sock_num);
    return ret;
}

/**
 * @brief Send data to a UDP socket with destination information
 *
 * @param sock_num  Socket number
 * @param buffer    Pointer to data to send
 * @param len       Length of data to send
 * @param dest_ip   Destination IP address (4 bytes)
 * @param dest_port Destination port
 * @return int32_t  Number of bytes sent, negative error code on failure
 */
int32_t w5500_socket_sendto(uint8_t sock_num, const uint8_t *buffer, uint16_t len,
                            const uint8_t *dest_ip, uint16_t dest_port)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_sendto: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    if (buffer == NULL)
    {
        DEBUG_PRINT("w5500_socket_sendto: Buffer is NULL\r\n");
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_socket_sendto: Sending %d bytes to %d.%d.%d.%d:%d on socket %d\r\n",
                len, dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3], dest_port, sock_num);
    // The 'sendto' function is from the WIZnet ioLibrary_Driver
    int32_t ret = sendto(sock_num, (uint8_t *)buffer, len, (uint8_t *)dest_ip, dest_port);
    if (ret < 0)
    {
        DEBUG_PRINT("w5500_socket_sendto: Failed to sendto data on socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    return ret;
}

/**
 * @brief Receive data from a UDP socket with source information
 *
 * @param sock_num  Socket number
 * @param buffer    Buffer to store received data
 * @param maxlen    Maximum length of buffer
 * @param src_ip    Buffer to store source IP (4 bytes)
 * @param src_port  Pointer to store source port
 * @return int32_t  Number of bytes received, negative error code on failure
 */
int32_t w5500_socket_recvfrom(uint8_t sock_num, uint8_t *buffer, uint16_t maxlen,
                              uint8_t *src_ip, uint16_t *src_port)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_recvfrom: Invalid socket number %d\r\n", sock_num);
        return W5500_SOCK_ERROR;
    }
    if (buffer == NULL)
    {
        DEBUG_PRINT("w5500_socket_recvfrom: Buffer is NULL\r\n");
        return W5500_SOCK_ERROR;
    }
    // The 'recvfrom' function is from the WIZnet ioLibrary_Driver
    int32_t ret = recvfrom(sock_num, buffer, maxlen, src_ip, src_port);
    if (ret < 0)
    {
        DEBUG_PRINT("w5500_socket_recvfrom: Failed to recvfrom data on socket %d, error %d\r\n", sock_num, ret);
        return W5500_SOCK_ERROR;
    }
    DEBUG_PRINT("w5500_socket_recvfrom: Received %d bytes from %d.%d.%d.%d:%d on socket %d\r\n",
                ret, src_ip[0], src_ip[1], src_ip[2], src_ip[3], *src_port, sock_num);
    return ret;
}

/*============================================================================*/
/* SOCKET STATUS                                      */
/*============================================================================*/

/**
 * @brief Get socket status
 *
 * @param sock_num  Socket number
 * @return uint8_t  Socket status code (see socket.h for Sn_SR_* defines)
 */
uint8_t w5500_socket_get_status(uint8_t sock_num)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_get_status: Invalid socket number %d\r\n", sock_num);
        return 0xFF; // Indicate error with an invalid status code
    }
    // getSn_SR is from wizchip_conf.h or socket.h in WIZnet library.
    uint8_t status = getSn_SR(sock_num);
    DEBUG_PRINT("w5500_socket_get_status: Socket %d current status: 0x%02X\r\n", sock_num, status);
    return status;
}

/**
 * @brief Get the amount of free transmit buffer size on a socket
 *
 * @param sock_num  Socket number
 * @return uint16_t Free TX buffer size in bytes
 */
uint16_t w5500_socket_get_tx_buf_free_size(uint8_t sock_num)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_get_tx_buf_free_size: Invalid socket number %d\r\n", sock_num);
        return 0;
    }
    // getSn_TX_FSR is from wizchip_conf.h or socket.h in WIZnet library.
    uint16_t size = getSn_TX_FSR(sock_num);
    DEBUG_PRINT("w5500_socket_get_tx_buf_free_size: Socket %d TX free size: %d\r\n", sock_num, size);
    return size;
}

/**
 * @brief Get the amount of received data in the RX buffer of a socket
 *
 * @param sock_num  Socket number
 * @return uint16_t RX buffer size in bytes
 */
uint16_t w5500_socket_get_rx_buf_size(uint8_t sock_num)
{
    if (sock_num >= W5500_MAX_SOCKET)
    {
        DEBUG_PRINT("w5500_socket_get_rx_buf_size: Invalid socket number %d\r\n", sock_num);
        return 0;
    }
    // getSn_RX_RSR is from wizchip_conf.h or socket.h in WIZnet library.
    uint16_t size = getSn_RX_RSR(sock_num);
    DEBUG_PRINT("w5500_socket_get_rx_buf_size: Socket %d RX size: %d\r\n", sock_num, size);
    return size;
}
