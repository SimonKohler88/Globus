/*
 * pic_buffer.h
 *
 *  Created on: 13.04.2025
 *      Author: skohl
 */

#ifndef MAIN_PIC_BUFFER_H_
#define MAIN_PIC_BUFFER_H_

#include "stdint.h"


enum
{
    BUFF_CTRL_HTTP_BUFF_1_JPEG_BUFF_2,
    BUFF_CTRL_HTTP_BUFF_2_JPEG_BUFF_1,
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
    uint8_t* buff_start_ptr;
    volatile uint32_t data_size;
    uint32_t buff_total_size;
    uint8_t valid;
} typedef eth_rx_buffer_t;

struct
{
    buff_status_t* status;
    volatile buffer_state_e buff_state;
    /* Buffer allocation instances*/
    eth_rx_buffer_t rx_buffer_1;
    eth_rx_buffer_t rx_buffer_2;
} typedef buffer_control_t;

void buff_ctrl_init( buffer_control_t* buff_ctrl, buff_status_t* status );
eth_rx_buffer_t* buff_ctrl_get_jpeg_src();
eth_rx_buffer_t* buff_ctrl_get_eth_buff();
void buff_ctrl_set_eth_buff_done( uint32_t valid );



#endif /* MAIN_PIC_BUFFER_H_ */
