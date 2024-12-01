/*
 * PSRAM_FIFO.h
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#ifndef MAIN_PSRAM_FIFO_H_
#define MAIN_PSRAM_FIFO_H_

#include "hw_settings.h"
#include "stdint.h"

struct
{
    uint8_t ready_4_fpga_frames;
    uint8_t free_frames;
    uint8_t current_frame_2_esp;
    uint8_t current_frame_2_fpga;
} typedef fifo_status_t;

struct
{
    uint8_t *frame_start_ptr;
    volatile uint8_t *current_ptr;
    volatile uint32_t size;
    uint32_t total_size;
} typedef fifo_frame_t;

void fifo_init( fifo_status_t *status );

uint8_t fifo_has_frame_4_fpga( void );
fifo_frame_t *fifo_get_frame_4_fpga( void );
void fifo_mark_frame_4_fpga_done( void );
uint8_t fifo_is_frame_2_fpga_in_progress( void );

uint8_t fifo_has_free_frame( void );
fifo_frame_t *fifo_get_free_frame( void );
void fifo_mark_free_frame_done( void );
void fifo_return_free_frame( void );

void fifo_update_stats( void );

fifo_frame_t* fifo_get_static_frame( void );
#endif /* MAIN_PSRAM_FIFO_H_ */
