/*
 * PSRAM_FIFO.h
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_PSRAM_FIFO_H_
#define MAIN_PSRAM_FIFO_H_

#include "stdint.h"
#include "hw_settings.h"

struct {
	uint8_t ready_4_fpga_frames;
	uint8_t free_frames;
	uint8_t errors;
} typedef fifo_status_t;



struct {
	uint8_t* frame_start_ptr;
	uint8_t* current_ptr;
	uint32_t size;
} typedef fifo_frame_t;



void fifo_init( fifo_status_t* status );


uint8_t fifo_has_frame_4_fpga( void );
uint8_t fifo_get_frame_4_fpga(fifo_frame_t* frame_info);
void fifo_mark_frame_4_fpga_done( void );

uint8_t fifo_has_free_frame( void );
uint8_t fifo_get_free_frame( fifo_frame_t* frame_info );
void fifo_mark_free_frame_done( void );

void fifo_update_stats( void );
#endif /* MAIN_PSRAM_FIFO_H_ */
