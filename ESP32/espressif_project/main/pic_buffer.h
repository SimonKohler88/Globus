/*
 * pic_buffer.h
 *
 *  Created on: 13.04.2025
 *      Author: skohl
 */

#ifndef MAIN_PIC_BUFFER_H_
#define MAIN_PIC_BUFFER_H_

#include "hw_settings.h"
#include "stdint.h"

struct
{
    uint8_t ready_4_fpga_frames;
    uint8_t free_frames;
    uint8_t current_frame_2_esp;
    uint8_t current_frame_2_fpga;
} typedef buff_status_t;

struct
{
    uint8_t* frame_start_ptr;
    volatile uint8_t* current_ptr;
    volatile uint32_t size;
    uint32_t total_size;
} typedef frame_unpacked_t;

struct
{
    buff_status_t* status;

    /* Buffer allocation instances*/
    frame_unpacked_t frame_unpacked;
    /*fpga dev purpose*/
    frame_unpacked_t static_pic_frame;

} typedef buffer_control_t;

void buff_ctrl_init( buffer_control_t* buff_ctrl, buff_status_t* status );
void buff_ctrl_copy_mem_protected( void* dst_ptr, const void* src_ptr, uint32_t size );
frame_unpacked_t* buff_ctrl_get_static_frame( void );
#endif /* MAIN_PIC_BUFFER_H_ */
