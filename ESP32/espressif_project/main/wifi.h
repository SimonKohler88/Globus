/*
 * wifi.h
 *
 *  Created on: 1 May 2021
 *      Author: cyril
 *
 *  copied and adapted: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include "freertos/FreeRTOS.h"

#include "PSRAM_FIFO.h"
#include "inttypes.h"


/* State of UDP Send Control */
enum
{
    WIFI_CTRL_IDLE,
    WIFI_CTRL_WAIT_FIRST_PKT,
    WIFI_CTRL_DATA_TRANSFER
};
typedef uint8_t WIFI_CTRL_STATE_t;

/* To communicate from UDP RX Task -> UDP Send Task (Ctrl) */
enum
{
    WIFI_FIRST_PACKET_RECEIVED,
    WIFI_DATA_TRANSFER_COMPLETED,
    WIFI_DATA_TRANSFER_ERROR,
};
typedef uint32_t WIFI_EVENT_MSG_t;

/* Internal WIFI Structure */
struct
{
    int UDP_socket;
    fifo_frame_t* current_frame_download;
    uint16_t tftp_block_number;
    uint16_t s_retry_num;
    uint8_t wifi_connected;
    WIFI_CTRL_STATE_t wifi_ctrl_state;

} typedef WIFI_STAT_INTERNAL_t;

/* TFTP Opcodes */
typedef enum
{
    eReadRequest = 1,
    eWriteRequest,
    eData,
    eAck,
    eError
} eTFTPOpcode_t;

/* Error codes from the RFC. */
typedef enum
{
    eFileNotFound = 1,
    eAccessViolation,
    eDiskFull,
    eIllegalTFTPOperation,
    eUnknownTransferID,
    eFileAlreadyExists
} eTFTPErrorCode_t;

/**
 * @brief Initializes the Wi-Fi receive functionality.
 *
 * Sets up the Wi-Fi station mode configuration, registers event handlers,
 * and starts the Wi-Fi. It configures necessary network settings for UDP communication.
 *
 * @details This function performs the following steps:
 * - Initializes the network interface.
 * - Creates the default event loop structure.
 * - Configures Wi-Fi station settings with predefined SSID and password.
 * - Registers event handlers for Wi-Fi and IP events.
 * - Starts the Wi-Fi service in station mode.
 * - Configures destination addresses for UDP and TFTP communication based on defined IPv4 and port settings.
 * - Logs the completion of initialization if debugging is enabled.
 *
 * Any errors in initializing components will be handled using the ESP_ERROR_CHECK mechanism,
 * which stops execution in case of failure.
 */
void wifi_receive_init( void );

/**
 * @brief Task to handle receiving UDP packets over Wi-Fi connection.
 *
 * @param pvParameters Pointer to the task parameters.
 *
 * This function runs continuously and performs the following operations:
 * - Checks if Wi-Fi is connected.
 * - Creates a UDP socket.
 * - Receives UDP packets as long as Wi-Fi remains connected.
 * - If a socket error occurs or if Wi-Fi disconnects, the socket is shut down, and the task waits before retrying.
 *
 * The task yields control using taskYIELD during packet reception to allow other tasks to run.
 * If HW_SETTINGS_DEBUG is enabled, socket creation and socket closing events are logged.
 */
void wifi_receive_udp_task( void* pvParameters );

/**
 * @brief Task responsible for sending UDP packets over WiFi.
 *
 * This function handles the sending of specific UDP packets that need to
 * be resent under certain conditions. It initializes a task command
 * queue and consistently checks this queue for commands to send UDP
 * packets. It ensures that sending only occurs if the device is connected
 * to a WiFi network and has a valid UDP socket.
 *
 * @param pvParameters Pointer to any parameters passed to the task upon creation.
 *                     Generally used in FreeRTOS tasks to pass runtime-specific
 *                     information, although not used explicitly in this function.
 */
void wifi_send_udp_task( void* pvParameters );

/**
 * @brief Checks if the Wi-Fi is currently connected.
 *
 * This function retrieves the current Wi-Fi connection status
 * by returning the value of the internal Wi-Fi connection flag.
 *
 * @return uint8_t A non-zero value indicates that the Wi-Fi is connected,
 * while zero indicates no active connection.
 */
uint8_t wifi_is_connected();

#endif /* MAIN_WIFI_H_ */
