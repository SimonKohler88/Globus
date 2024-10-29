/*
 * rpi_interface.h
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_RPI_INTERFACE_H_
#define MAIN_RPI_INTERFACE_H_

#include "stdint.h"

#define MAX_STATUS_STRUCTS 10
#define MAX_PARAM_STRUCTS  5

void register_status_struct( void* if_struct, uint32_t size );

uint32_t get_status_data( void* data_ptr );

#endif /* MAIN_RPI_INTERFACE_H_ */
