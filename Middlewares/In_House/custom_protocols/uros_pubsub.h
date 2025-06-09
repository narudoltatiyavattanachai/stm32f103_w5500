/**
 * @file uros_publisher.h
 * @brief User-friendly micro-ROS publisher API for STM32 with W5500
 * 
 * @details This header provides a simplified interface for creating and using
 *          micro-ROS publishers on STM32 microcontrollers with W5500 Ethernet.
 *          It abstracts away the complexity of the micro-ROS API to provide
 *          simple functions for publishing ROS messages.
 * 
 * @author Your Name
 * @date 2025-06-09
 */

#ifndef _UROS_PUBLISHER_H_
#define _UROS_PUBLISHER_H_

#include <stdint.h>
#include <stdbool.h>

/* Only compile if micro-ROS support is enabled */
#ifdef RMW_UXRCE_TRANSPORT_CUSTOM

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

/**
 * @brief Initialize the micro-ROS publisher module
 * 
 * @details Initializes the micro-ROS context, node, and support. This function
 *          must be called before creating any publishers, typically early in the
 *          application startup after the transport has been initialized.
 * 
 * @param agent_ip IP address of the micro-ROS agent (4 bytes)
 * @param agent_port Port number of the micro-ROS agent
 * @param node_name Name of the ROS node to create
 * @param namespace Namespace for the ROS node (use "" for root namespace)
 * 
 * @return true if initialization successful, false otherwise
 */
bool uros_publisher_init(const uint8_t agent_ip[4], uint16_t agent_port, 
                        const char* node_name, const char* namespace);

/**
 * @brief Create a generic micro-ROS publisher
 * 
 * @details Creates a publisher for a specific message type. This function uses 
 *          the type support provided by micro-ROS for the specific message type.
 *          The caller must provide an allocated publisher object and type support.
 * 
 * @param publisher Pointer to an rcl_publisher_t object to initialize
 * @param topic_name Name of the ROS topic to publish on
 * @param type_support Type support for the message to be published
 * @param qos_profile Quality of service profile (use RCLD_DEFAULT_* constants)
 * 
 * @return true if publisher creation successful, false otherwise
 * 
 * @note This is a generic function. For specific message types, use the
 *       convenience functions provided for common message types.
 */
bool uros_publisher_create(rcl_publisher_t* publisher, const char* topic_name, 
                          const rosidl_message_type_support_t* type_support, 
                          const rmw_qos_profile_t* qos_profile);

/**
 * @brief Publish a message using a previously created publisher
 * 
 * @details Publishes a message to the ROS topic associated with the publisher.
 *          The caller must provide a properly initialized message of the correct
 *          type for the publisher.
 * 
 * @param publisher Pointer to an initialized rcl_publisher_t object
 * @param message Pointer to the message to publish
 * 
 * @return true if publish successful, false otherwise
 */
bool uros_publisher_publish(rcl_publisher_t* publisher, void* message);

/**
 * @brief Spin once to process micro-ROS communications
 * 
 * @details This function should be called regularly to process incoming
 *          messages and maintain the connection to the micro-ROS agent.
 *          It can be called in the main loop or from a timer callback.
 * 
 * @param timeout_ms Maximum time to wait for events in milliseconds
 * 
 * @return true if spin successful, false otherwise
 */
bool uros_publisher_spin_once(uint32_t timeout_ms);

/**
 * @brief Check if connected to the micro-ROS agent
 * 
 * @details This function checks if the connection to the micro-ROS agent
 *          is active and working correctly.
 * 
 * @return true if connected, false otherwise
 */
bool uros_publisher_is_connected(void);

/**
 * @brief Clean up the micro-ROS publisher module
 * 
 * @details Frees all resources associated with the micro-ROS publishers,
 *          nodes, and context. Should be called when the application is
 *          shutting down or no longer needs micro-ROS publishing.
 * 
 * @param publisher Pointer to an initialized rcl_publisher_t object to clean up
 * 
 * @return true if cleanup successful, false otherwise
 */
bool uros_publisher_cleanup(rcl_publisher_t* publisher);

/**
 * @brief Convenience function to create a standard Int32 publisher
 * 
 * @details Creates a publisher for the std_msgs/Int32 message type.
 * 
 * @param publisher Pointer to an rcl_publisher_t object to initialize
 * @param topic_name Name of the ROS topic to publish on
 * 
 * @return true if publisher creation successful, false otherwise
 */
bool uros_publisher_create_int32(rcl_publisher_t* publisher, const char* topic_name);

/**
 * @brief Convenience function to create a standard Float32 publisher
 * 
 * @details Creates a publisher for the std_msgs/Float32 message type.
 * 
 * @param publisher Pointer to an rcl_publisher_t object to initialize
 * @param topic_name Name of the ROS topic to publish on
 * 
 * @return true if publisher creation successful, false otherwise
 */
bool uros_publisher_create_float32(rcl_publisher_t* publisher, const char* topic_name);

/**
 * @brief Convenience function to create a standard String publisher
 * 
 * @details Creates a publisher for the std_msgs/String message type.
 * 
 * @param publisher Pointer to an rcl_publisher_t object to initialize
 * @param topic_name Name of the ROS topic to publish on
 * 
 * @return true if publisher creation successful, false otherwise
 */
bool uros_publisher_create_string(rcl_publisher_t* publisher, const char* topic_name);

/**
 * @brief Convenience function to publish an Int32 message
 * 
 * @details Publishes an integer value to a std_msgs/Int32 topic.
 * 
 * @param publisher Pointer to an initialized Int32 publisher
 * @param value Integer value to publish
 * 
 * @return true if publish successful, false otherwise
 */
bool uros_publisher_publish_int32(rcl_publisher_t* publisher, int32_t value);

/**
 * @brief Convenience function to publish a Float32 message
 * 
 * @details Publishes a float value to a std_msgs/Float32 topic.
 * 
 * @param publisher Pointer to an initialized Float32 publisher
 * @param value Float value to publish
 * 
 * @return true if publish successful, false otherwise
 */
bool uros_publisher_publish_float32(rcl_publisher_t* publisher, float value);

/**
 * @brief Convenience function to publish a String message
 * 
 * @details Publishes a string value to a std_msgs/String topic.
 * 
 * @param publisher Pointer to an initialized String publisher
 * @param text String to publish
 * 
 * @return true if publish successful, false otherwise
 */
bool uros_publisher_publish_string(rcl_publisher_t* publisher, const char* text);

#endif /* RMW_UXRCE_TRANSPORT_CUSTOM */

#endif /* _UROS_PUBLISHER_H_ */