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

#include "inttypes.h"


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
