/**
 * @file w5500_transport.c
 * @brief Unified W5500 transport implementation for both standard usage and micro-ROS integration
 * @author Your Name
 * @date 2025-06-09
 */

#include "w5500_transport.h"
#include "socket.h"
#include "wizchip_conf.h"
#include <stdint.h>
#include <string.h>

// Used for HAL functions
#include "stm32f1xx_hal.h"

// Include micro-ROS headers when building with micro-ROS support
#ifdef RMW_UXRCE_TRANSPORT_CUSTOM
#include <rmw_microros/rmw_microros.h>
#include <uxr/client/transport.h>
#endif

/* Private defines -------------------------------------------------------------*/
#define W5500_SPI_TIMEOUT      1000
#define W5500_SOCKET_NUMBER    0
#define W5500_SOCKET_BUFSIZE   2048
#define TCP_PROTOCOL           1
#define DEBUG_PRINT(...)       // Enable if needed

/* Private variables ----------------------------------------------------------*/
static uint8_t agent_ip[4] = {0};
static uint16_t agent_port = 0;
static uint16_t local_port = 0;
static uint8_t socket_status = 0;
static SPI_HandleTypeDef *hw_spi;

/* Function prototypes --------------------------------------------------------*/
static bool w5500_hw_init(void);
static bool w5500_network_init(void);
static bool w5500_socket_connect(void);

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM
// micro-ROS specific function prototypes
static int8_t w5500_transport_open(struct rmw_uros_transport * transport);
static bool w5500_transport_close(struct rmw_uros_transport * transport);
static size_t w5500_transport_write(struct rmw_uros_transport * transport, 
                                   const uint8_t *buf, size_t len);
static size_t w5500_transport_read(struct rmw_uros_transport * transport, 
                                  uint8_t *buf, size_t len);
#endif

/* Hardware-specific functions - adapt for your platform ----------------------*/

static void w5500_select(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

static void w5500_deselect(void)
{
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

static uint8_t w5500_spi_read_write(uint8_t data)
{
    uint8_t rx_data = 0;
    
    // Implement SPI transmit and receive byte
    if (HAL_SPI_TransmitReceive(hw_spi, &data, &rx_data, 1, W5500_SPI_TIMEOUT) != HAL_OK)
    {
        return 0; // Error handling
    }
    
    return rx_data;
}

static void w5500_spi_read_burst(uint8_t *buf, uint16_t len)
{
    // Implement SPI burst read
    HAL_SPI_Receive(hw_spi, buf, len, W5500_SPI_TIMEOUT);
}

static void w5500_spi_write_burst(uint8_t *buf, uint16_t len)
{
    // Implement SPI burst write
    HAL_SPI_Transmit(hw_spi, buf, len, W5500_SPI_TIMEOUT);
}

/* W5500 Transport Implementation --------------------------------------------*/

bool w5500_init(const uint8_t mac[6], const uint8_t ip[4], 
                const uint8_t subnet[4], const uint8_t gateway[4])
{
    // Variables
    uint8_t tmp;
    wiz_NetInfo net_info = {0};
    
    // Initialize hardware SPI
    if (!w5500_hw_init())
    {
        return false;
    }
    
    // Register SPI functions and initialize W5500 chip
    reg_wizchip_cs_cbfunc(w5500_select, w5500_deselect);
    reg_wizchip_spi_cbfunc(w5500_spi_read_write, w5500_spi_write_burst, w5500_spi_read_burst);
    
    wizchip_init(NULL, NULL);
    
    // Check if W5500 is accessible
    if (getVERSIONR() != 0x04)
    {
        DEBUG_PRINT("W5500 not detected\r\n");
        return false;
    }
    
    // Set MAC, IP, Gateway and Subnet mask
    memcpy(net_info.mac, mac, 6);
    memcpy(net_info.ip, ip, 4);
    memcpy(net_info.sn, subnet, 4);
    memcpy(net_info.gw, gateway, 4);
    
    wizchip_setnetinfo(&net_info);
    
    // Set the socket buffer size
    uint8_t tx_mem_conf[8] = {2, 2, 2, 2, 2, 2, 2, 2};  // 2KB per socket
    uint8_t rx_mem_conf[8] = {2, 2, 2, 2, 2, 2, 2, 2};  // 2KB per socket
    wizchip_setbutfsize(tx_mem_conf, rx_mem_conf);
    
    DEBUG_PRINT("W5500 initialized successfully\r\n");
    return true;
}

bool w5500_socket_open(const uint8_t agent_ip_addr[4], uint16_t agent_port_num, uint16_t local_port_num)
{
    // Store agent information
    memcpy(agent_ip, agent_ip_addr, 4);
    agent_port = agent_port_num;
    local_port = local_port_num;
    
    // Close socket if it was previously open
    close(W5500_SOCKET_NUMBER);
    
    // Create TCP socket
    if (socket(W5500_SOCKET_NUMBER, Sn_MR_TCP, local_port, 0) != W5500_SOCKET_NUMBER)
    {
        DEBUG_PRINT("Failed to create socket\r\n");
        return false;
    }
    
    // Connect to agent
    if (connect(W5500_SOCKET_NUMBER, agent_ip, agent_port) != SOCK_OK)
    {
        DEBUG_PRINT("Failed to connect to agent\r\n");
        close(W5500_SOCKET_NUMBER);
        return false;
    }
    
    // Connection established
    DEBUG_PRINT("Connected to agent\r\n");
    socket_status = 1;
    return true;
}

bool w5500_socket_close(void)
{
    if (socket_status)
    {
        disconnect(W5500_SOCKET_NUMBER);
        close(W5500_SOCKET_NUMBER);
        socket_status = 0;
        DEBUG_PRINT("Socket closed\r\n");
        return true;
    }
    return false;
}

bool w5500_poll(uint32_t timeout_ms)
{
    // Variables
    uint8_t status;
    uint32_t start_time = HAL_GetTick();
    
    // Check socket status with timeout
    do
    {
        status = getSn_SR(W5500_SOCKET_NUMBER);
        
        // Check if socket is established
        if (status == SOCK_ESTABLISHED)
        {
            // Check if data available
            if (getSn_RX_RSR(W5500_SOCKET_NUMBER) > 0)
            {
                return true;
            }
        }
        else if (status == SOCK_CLOSED)
        {
            // Socket closed unexpectedly, try to reconnect
            socket_status = 0;
            w5500_socket_open(agent_ip, agent_port, local_port);
            return false;
        }
        
        // Small delay
        HAL_Delay(1);
        
    } while ((HAL_GetTick() - start_time) < timeout_ms);
    
    return false; // Timeout occurred
}

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM
/* Micro-ROS Transport Interface Implementation ------------------------------*/

static int8_t w5500_transport_open(struct rmw_uros_transport * transport)
{
    (void) transport;
    return socket_status ? RMW_UROS_ERROR_OK : RMW_UROS_ERROR_GENERAL;
}

static bool w5500_transport_close(struct rmw_uros_transport * transport)
{
    (void) transport;
    return w5500_socket_close();
}

static size_t w5500_transport_write(struct rmw_uros_transport * transport, 
                                  const uint8_t *buf, size_t len)
{
    (void) transport;
    
    int32_t sent_len = 0;
    uint8_t status = getSn_SR(W5500_SOCKET_NUMBER);
    
    if (status == SOCK_ESTABLISHED)
    {
        sent_len = send(W5500_SOCKET_NUMBER, (uint8_t*)buf, len);
        if (sent_len > 0)
        {
            return (size_t)sent_len;
        }
    }
    
    return 0;
}

static size_t w5500_transport_read(struct rmw_uros_transport * transport, 
                                 uint8_t *buf, size_t len)
{
    (void) transport;
    
    int32_t recv_len = 0;
    uint16_t rx_size = 0;
    uint8_t status = getSn_SR(W5500_SOCKET_NUMBER);
    
    if (status == SOCK_ESTABLISHED)
    {
        rx_size = getSn_RX_RSR(W5500_SOCKET_NUMBER);
        if (rx_size > 0)
        {
            recv_len = recv(W5500_SOCKET_NUMBER, buf, len);
            if (recv_len > 0)
            {
                return (size_t)recv_len;
            }
        }
    }
    
    return 0;
}

/* micro-ROS Custom Transport API Implementation ----------------------------*/

bool cubemx_transport_open(struct uxrCustomTransport * transport)
{
    uint8_t *args = (uint8_t*)transport->args;
    if (args != NULL && args[0] != 0)
    {
        memcpy(agent_ip, args, 4);
        agent_port = (args[4] << 8) | args[5];
    }
    if (!w5500_hw_init())
    {
        return false;
    }
    if (!w5500_network_init())
    {
        return false;
    }
    return w5500_socket_connect();
}

bool cubemx_transport_close(struct uxrCustomTransport * transport)
{
    (void) transport;
    return w5500_socket_close();
}

size_t cubemx_transport_write(struct uxrCustomTransport* transport, uint8_t * buf, size_t len, uint8_t * err)
{
    (void) transport;
    
    uint8_t status = getSn_SR(W5500_SOCKET_NUMBER);
    if (status == SOCK_ESTABLISHED)
    {
        int32_t sent_len = send(W5500_SOCKET_NUMBER, buf, len);
        if (sent_len > 0)
        {
            *err = 0;
            return (size_t)sent_len;
        }
    }
    else if (status == SOCK_CLOSED)
    {
        // Try to reconnect
        if (w5500_socket_connect())
        {
            // Retry sending after reconnection
            int32_t sent_len = send(W5500_SOCKET_NUMBER, buf, len);
            if (sent_len > 0)
            {
                *err = 0;
                return (size_t)sent_len;
            }
        }
    }
    
    *err = 1; // Indicate error
    return 0;
}

size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err)
{
    (void) transport;
    
    uint8_t status = getSn_SR(W5500_SOCKET_NUMBER);
    if (status == SOCK_ESTABLISHED)
    {
        // Poll for data with timeout
        uint32_t start_time = HAL_GetTick();
        while ((HAL_GetTick() - start_time) < (uint32_t)timeout)
        {
            uint16_t rx_size = getSn_RX_RSR(W5500_SOCKET_NUMBER);
            if (rx_size > 0)
            {
                int32_t recv_len = recv(W5500_SOCKET_NUMBER, buf, len);
                if (recv_len > 0)
                {
                    *err = 0;
                    return (size_t)recv_len;
                }
                break;
            }
            HAL_Delay(1);
        }
    }
    else if (status == SOCK_CLOSED)
    {
        // Try to reconnect
        w5500_socket_connect();
    }
    
    *err = 1; // Indicate error or timeout
    return 0;
}
#endif

/* Hardware Initialization Function ------------------------------------------*/

static bool w5500_hw_init(void)
{
    // Get SPI handle from HAL (assuming it's already initialized elsewhere)
    hw_spi = &hspi1; // Set this to your SPI handle
    
    // Hardware reset of W5500
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(50); // Wait for W5500 to initialize
    
    return true;
}

/* Public Functions ----------------------------------------------------------*/

/**
 * @brief Performs network initialization for W5500
 * Default network settings used if not previously configured
 * @return true if successful, false otherwise
 */
static bool w5500_network_init(void)
{
    static const uint8_t default_mac[6] = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56};
    static const uint8_t default_ip[4] = {192, 168, 1, 100};
    static const uint8_t default_subnet[4] = {255, 255, 255, 0};
    static const uint8_t default_gateway[4] = {192, 168, 1, 1};
    
    wiz_NetInfo net_info = {0};
    
    // Set MAC, IP, Gateway and Subnet mask
    memcpy(net_info.mac, default_mac, 6);
    memcpy(net_info.ip, default_ip, 4);
    memcpy(net_info.sn, default_subnet, 4);
    memcpy(net_info.gw, default_gateway, 4);
    
    wizchip_setnetinfo(&net_info);
    
    // Set the socket buffer size
    uint8_t tx_mem_conf[8] = {2, 2, 2, 2, 2, 2, 2, 2};  // 2KB per socket
    uint8_t rx_mem_conf[8] = {2, 2, 2, 2, 2, 2, 2, 2};  // 2KB per socket
    wizchip_setbutfsize(tx_mem_conf, rx_mem_conf);
    
    DEBUG_PRINT("W5500 network initialized\r\n");
    return true;
}

/**
 * @brief Connect to the micro-ROS agent using TCP
 * Uses the previously stored agent IP and port
 * @return true if connected successfully, false otherwise
 */
static bool w5500_socket_connect(void)
{
    // Use default local port if none specified
    if (local_port == 0) {
        local_port = 8888; // Default local port
    }
    
    // If agent IP not set, use defaults
    if (agent_ip[0] == 0 && agent_ip[1] == 0 && agent_ip[2] == 0 && agent_ip[3] == 0)
    {
        // Default agent IP and port (localhost, port 8888)
        agent_ip[0] = 192;
        agent_ip[1] = 168;
        agent_ip[2] = 1;
        agent_ip[3] = 10;
        agent_port = 8888;
    }
    
    // Create TCP socket
    if (socket(W5500_SOCKET_NUMBER, Sn_MR_TCP, local_port, 0) != W5500_SOCKET_NUMBER)
    {
        DEBUG_PRINT("Failed to create socket\r\n");
        return false;
    }
    
    // Connect to agent
    if (connect(W5500_SOCKET_NUMBER, agent_ip, agent_port) != SOCK_OK)
    {
        DEBUG_PRINT("Failed to connect to agent\r\n");
        close(W5500_SOCKET_NUMBER);
        return false;
    }
    
    // Connection established
    DEBUG_PRINT("Connected to agent\r\n");
    socket_status = 1;
    return true;
}

#ifdef RMW_UXRCE_TRANSPORT_CUSTOM
rmw_uros_transport_params_t w5500_get_transport(void)
{
    rmw_uros_transport_params_t params;
    
    params.framing = MICRO_ROS_FRAMING_NONE;
    params.open_cb = w5500_transport_open;
    params.close_cb = w5500_transport_close;
    params.write_cb = w5500_transport_write;
    params.read_cb = w5500_transport_read;
    
    return params;
}
#endif