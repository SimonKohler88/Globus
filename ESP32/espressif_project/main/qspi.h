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

// TODO: put frame request pin to interrupt and call qspi_DMA_write
struct 
{
	uint16_t missed_frames;
	uint16_t missed_spi_transfers;
	
} typedef qspi_status_t;

void qspi_init( qspi_status_t *status_ptr );
//void qspi_DMA_write( void );
//void qspi_DMA_write_debug_test( uint8_t* buffer, uint8_t size );

BaseType_t qspi_request_frame( void );

void fpga_qspi_task( void* pvParameter );


#endif /* MAIN_QSPI_H_ */
