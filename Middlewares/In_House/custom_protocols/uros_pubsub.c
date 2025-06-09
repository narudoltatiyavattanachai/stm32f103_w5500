/**
 * @file uros_publisher.c
 * @brief User-friendly micro-ROS publisher API implementation
 * 
 * @details Implementation of the simplified interface for creating and using
 *          micro-ROS publishers on STM32 microcontrollers with W5500 Ethernet.
 * 
 * @author Your Name
 * @date 2025-06-09
 */

/* Includes ------------------------------------------------------------------*/
#include "uros_publisher.h"
#include "uros_transport.h"

/* Only compile if micro-ROS support is enabled */
#ifdef RMW_UXRCE_TRANSPORT_CUSTOM

#include <rcutils/allocator.h>
#include <rmw_microros/rmw_microros.h>
#include <stm32f1xx_hal.h> /* For HAL_GetTick() */

/* Standard message headers */
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/float32.h>
#include <std_msgs/msg/string.h>

/* Debug print macro - comment out to disable debug prints */
#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
#include <stdio.h>  /* For printf */
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* No print */
#endif

/* Private variables ---------------------------------------------------------*/
static rcl_allocator_t allocator;
static rclc_support_t support;
static rcl_node_t node;
static rclc_executor_t executor;
static bool uros_initialized = false;
static bool uros_connected = false;

/**
 * @brief Initialize the micro-ROS publisher module
 */
bool uros_publisher_init(const uint8_t agent_ip[4], uint16_t agent_port, 
                        const char* node_name, const char* namespace)
{
    /* Initialize the micro-ROS transport */
    if (!uros_transport_init(agent_ip, agent_port)) {
        DEBUG_PRINT("Failed to initialize micro-ROS transport\r\n");
        return false;
    }
    
    /* Get transport parameters */
    rmw_uros_transport_params_t params = uros_get_transport();
    rmw_uros_set_custom_transport(
        params.framing,
        params.args,
        params.open_cb,
        params.close_cb,
        params.write_cb,
        params.read_cb
    );
    
    /* Initialize micro-ROS allocator */
    allocator = rcutils_get_default_allocator();
    
    /* Initialize micro-ROS support */
    rcl_ret_t ret = rclc_support_init(&support, 0, NULL, &allocator);
    if (ret != RCL_RET_OK) {
        DEBUG_PRINT("Failed to initialize micro-ROS support (%d)\r\n", ret);
        return false;
    }
    
    /* Create micro-ROS node */
    ret = rclc_node_init_default(&node, node_name, namespace, &support);
    if (ret != RCL_RET_OK) {
        DEBUG_PRINT("Failed to initialize micro-ROS node (%d)\r\n", ret);
        return false;
    }
    
    /* Create micro-ROS executor */
    ret = rclc_executor_init(&executor, &support.context, 1, &allocator);
    if (ret != RCL_RET_OK) {
        DEBUG_PRINT("Failed to initialize micro-ROS executor (%d)\r\n", ret);
        return false;
    }
    
    uros_initialized = true;
    uros_connected = true;
    DEBUG_PRINT("micro-ROS publisher module initialized\r\n");
    return true;
}

/**
 * @brief Create a generic micro-ROS publisher
 */
bool uros_publisher_create(rcl_publisher_t* publisher, const char* topic_name, 
                          const rosidl_message_type_support_t* type_support, 
                          const rmw_qos_profile_t* qos_profile)
{
    if (!uros_initialized) {
        DEBUG_PRINT("micro-ROS not initialized. Call uros_publisher_init() first\r\n");
        return false;
    }
    
    rcl_publisher_options_t options = rcl_publisher_get_default_options();
    options.qos = *qos_profile;
    
    rcl_ret_t ret = rcl_publisher_init(publisher, &node, type_support, topic_name, &options);
    if (ret != RCL_RET_OK) {
        DEBUG_PRINT("Failed to create publisher for topic %s (%d)\r\n", topic_name, ret);
        return false;
    }
    
    DEBUG_PRINT("Created publisher for topic '%s'\r\n", topic_name);
    return true;
}

/**
 * @brief Publish a message using a previously created publisher
 */
bool uros_publisher_publish(rcl_publisher_t* publisher, void* message)
{
    if (!uros_initialized) {
        DEBUG_PRINT("micro-ROS not initialized. Call uros_publisher_init() first\r\n");
        return false;
    }
    
    rcl_ret_t ret = rcl_publish(publisher, message, NULL);
    if (ret != RCL_RET_OK) {
        DEBUG_PRINT("Failed to publish message (%d)\r\n", ret);
        return false;
    }
    
    return true;
}

/**
 * @brief Spin once to process micro-ROS communications
 */
bool uros_publisher_spin_once(uint32_t timeout_ms)
{
    if (!uros_initialized) {
        DEBUG_PRINT("micro-ROS not initialized. Call uros_publisher_init() first\r\n");
        return false;
    }
    
    rcl_ret_t ret = rclc_executor_spin_some(&executor, RCL_MS_TO_NS(timeout_ms));
    if (ret != RCL_RET_OK) {
        if (ret == RCL_RET_TIMEOUT) {
            /* Timeout is normal, not an error */
            return true;
        }
        
        DEBUG_PRINT("Failed to spin micro-ROS executor (%d)\r\n", ret);
        uros_connected = false;
        return false;
    }
    
    uros_connected = true;
    return true;
}

/**
 * @brief Check if connected to the micro-ROS agent
 */
bool uros_publisher_is_connected(void)
{
    if (!uros_initialized) {
        return false;
    }
    
    return uros_connected;
}

/**
 * @brief Clean up the micro-ROS publisher module
 */
bool uros_publisher_cleanup(rcl_publisher_t* publisher)
{
    if (!uros_initialized) {
        return false;
    }
    
    rcl_ret_t ret;
    
    /* Destroy publisher if provided */
    if (publisher != NULL) {
        ret = rcl_publisher_fini(publisher, &node);
        if (ret != RCL_RET_OK) {
            DEBUG_PRINT("Failed to destroy publisher (%d)\r\n", ret);
            return false;
        }
    }
    
    /* Cleanup other resources only if publisher is NULL (indicating full cleanup) */
    if (publisher == NULL) {
        /* Destroy executor */
        ret = rclc_executor_fini(&executor);
        if (ret != RCL_RET_OK) {
            DEBUG_PRINT("Failed to destroy executor (%d)\r\n", ret);
            return false;
        }
        
        /* Destroy node */
        ret = rcl_node_fini(&node);
        if (ret != RCL_RET_OK) {
            DEBUG_PRINT("Failed to destroy node (%d)\r\n", ret);
            return false;
        }
        
        /* Destroy support */
        ret = rclc_support_fini(&support);
        if (ret != RCL_RET_OK) {
            DEBUG_PRINT("Failed to destroy support (%d)\r\n", ret);
            return false;
        }
        
        uros_initialized = false;
        uros_connected = false;
        DEBUG_PRINT("micro-ROS publisher module cleaned up\r\n");
    }
    
    return true;
}

/**
 * @brief Convenience function to create a standard Int32 publisher
 */
bool uros_publisher_create_int32(rcl_publisher_t* publisher, const char* topic_name)
{
    return uros_publisher_create(
        publisher,
        topic_name,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        &rmw_qos_profile_default
    );
}

/**
 * @brief Convenience function to create a standard Float32 publisher
 */
bool uros_publisher_create_float32(rcl_publisher_t* publisher, const char* topic_name)
{
    return uros_publisher_create(
        publisher,
        topic_name,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
        &rmw_qos_profile_default
    );
}

/**
 * @brief Convenience function to create a standard String publisher
 */
bool uros_publisher_create_string(rcl_publisher_t* publisher, const char* topic_name)
{
    return uros_publisher_create(
        publisher,
        topic_name,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        &rmw_qos_profile_default
    );
}

/**
 * @brief Convenience function to publish an Int32 message
 */
bool uros_publisher_publish_int32(rcl_publisher_t* publisher, int32_t value)
{
    std_msgs__msg__Int32 msg;
    msg.data = value;
    return uros_publisher_publish(publisher, &msg);
}

/**
 * @brief Convenience function to publish a Float32 message
 */
bool uros_publisher_publish_float32(rcl_publisher_t* publisher, float value)
{
    std_msgs__msg__Float32 msg;
    msg.data = value;
    return uros_publisher_publish(publisher, &msg);
}

/**
 * @brief Convenience function to publish a String message
 */
bool uros_publisher_publish_string(rcl_publisher_t* publisher, const char* text)
{
    std_msgs__msg__String msg;
    
    /* Initialize string message */
    rosidl_runtime_c__String__init(&msg.data);
    
    /* Copy provided text to message */
    if (!rosidl_runtime_c__String__assign(&msg.data, text)) {
        DEBUG_PRINT("Failed to assign string value\r\n");
        rosidl_runtime_c__String__fini(&msg.data);
        return false;
    }
    
    /* Publish the message */
    bool success = uros_publisher_publish(publisher, &msg);
    
    /* Clean up string resources */
    rosidl_runtime_c__String__fini(&msg.data);
    
    return success;
}

/* Usage example (for reference only, not part of implementation):

int main(void)
{
    // Initialize W5500 and network
    uint8_t ip[4] = {192, 168, 1, 100};
    uint8_t gateway[4] = {192, 168, 1, 1};
    uint8_t subnet[4] = {255, 255, 255, 0};
    uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x01, 0x02, 0x03};
    uint8_t agent_ip[4] = {192, 168, 1, 10};
    
    w5500_init(&hspi2, mac, ip, subnet, gateway);
    
    // Initialize micro-ROS publisher
    uros_publisher_init(agent_ip, 8888, "stm32_node", "");
    
    // Create publishers
    rcl_publisher_t int_pub, float_pub, string_pub;
    uros_publisher_create_int32(&int_pub, "stm32/counter");
    uros_publisher_create_float32(&float_pub, "stm32/temperature");
    uros_publisher_create_string(&string_pub, "stm32/status");
    
    int counter = 0;
    
    while (1) {
        // Publish data periodically
        uros_publisher_publish_int32(&int_pub, counter++);
        uros_publisher_publish_float32(&float_pub, 25.5f);
        uros_publisher_publish_string(&string_pub, "Online");
        
        // Process micro-ROS communications
        uros_publisher_spin_once(10);
        
        HAL_Delay(1000);
    }
}
*/

#endif /* RMW_UXRCE_TRANSPORT_CUSTOM */