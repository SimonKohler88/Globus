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

enum
{
    BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2,
    BUFF_CTRL_WIFI_QSI_BUFF_2_JPEG_BUFF_1,
};
typedef uint8_t buffer_state_e;
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
    uint8_t* current_ptr;
    volatile uint32_t size;
    uint32_t total_size;
} typedef frame_unpacked_t;

struct
{
    uint8_t* buff_start_ptr;
    volatile uint32_t data_size;
    uint32_t buff_total_size;
} typedef eth_rx_buffer_t;

struct
{
    buff_status_t* status;
    volatile buffer_state_e buff_state;
    /* Buffer allocation instances*/
    eth_rx_buffer_t rx_buffer_1;
    uint8_t rx_buffer_1_valid;
    eth_rx_buffer_t rx_buffer_2;
    uint8_t rx_buffer_2_valid;
    frame_unpacked_t frame_unpacked_1;
    uint8_t frame_unpacked_1_valid;
    frame_unpacked_t frame_unpacked_2;
    uint8_t frame_unpacked_2_valid;

    /*fpga dev purpose*/
    frame_unpacked_t static_pic_frame;

} typedef buffer_control_t;

void buff_ctrl_init( buffer_control_t* buff_ctrl, buff_status_t* status );
void buff_ctrl_copy_mem_protected( void* dst_ptr, const void* src_ptr, uint32_t size );

void buff_ctrl_toggle_buff();
eth_rx_buffer_t* buff_ctrl_get_jpeg_src();
frame_unpacked_t* buff_ctrl_get_jpeg_dst();
void buff_ctrl_set_jpec_dst_done( uint8_t valid );
eth_rx_buffer_t* buff_ctrl_get_eth_buff();
void buff_ctrl_set_eth_buff_done( uint32_t valid );
frame_unpacked_t* buff_ctrl_get_qspi_src();

frame_unpacked_t* buff_ctrl_get_static_frame( void );



#endif /* MAIN_PIC_BUFFER_H_ */
