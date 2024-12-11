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

/**
 * Registers a status structure for monitoring or processing.
 *
 * @param if_struct A pointer to the status structure to be registered. This pointer should point to a valid memory
 *                  location containing the structure data.
 * @param size The size of the status structure in bytes. This helps in managing the memory occupied by the structure
 *             and should be accurately specified.
 */
void register_status_struct( void* if_struct, uint32_t size );

/**
 * Aggregates and copies registered status structures into the provided data pointer.
 *
 * This function iterates over all registered status structures, copying each one
 * into the provided data pointer. The total size of the copied data is returned.
 *
 * @param data_ptr A pointer to the memory location where the status data will be copied.
 *                 It's assumed that enough memory has been allocated to store all the
 *                 registered status data.
 *
 * @return The total size, in bytes, of the data copied into data_ptr.
 */
uint32_t get_status_data( void* data_ptr );

#endif /* MAIN_RPI_INTERFACE_H_ */
