/*
 * qspi.h
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#ifndef MAIN_QSPI_H_
#define MAIN_QSPI_H_

#include "stdint.h"
#include "freertos/FreeRTOS.h"

#include"status_control_task.h"


struct 
{
	uint16_t missed_frames;
	uint16_t missed_spi_transfers;
	
} typedef qspi_status_t;

/**
 * @brief Initializes the QSPI interface and configures the necessary hardware settings.
 *
 * This function sets up the QSPI interface by initializing the SPI bus and adding
 * an SPI device with the specified configuration. It configures the bus and device
 * according to the predefined settings required for operation.
 *
 * @param status_ptr Pointer to a qspi_status_t structure where the status of the
 *                   QSPI initialization will be stored.
 */
void qspi_init( qspi_status_t* status_ptr , task_handles_t* task_handles);

/**
 * @brief Initiates a QSPI frame request signal from an ISR.
 *
 * This function is designed to be called from an interrupt service routine (ISR).
 * It checks if the internal QSPI task handle is not NULL, and then sends a
 * notification to the task via `xTaskNotifyIndexedFromISR`. This mechanism is
 * intended to alert the task to start processing a QSPI frame.
 *
 * @return A BaseType_t indicating whether a context switch is required
 * when the ISR exits. The function returns `pdTRUE` if a higher priority
 * task was woken, `pdFALSE` otherwise.
 */
BaseType_t qspi_request_frame( void );

/**
 * Performs the main task for handling QSPI communication with an FPGA device.
 * This function is expected to run as a FreeRTOS task and will continuously
 * operate within an infinite loop.
 *
 * @param pvParameter A pointer to the parameters passed to the task. This parameter
 *        can be used to pass context or configuration data to the task, typically cast
 *        to a specific type relevant to the task.
 */
void fpga_qspi_task( void* pvParameter );


#endif /* MAIN_QSPI_H_ */
