/*
 * PSRAM_FIFO.c
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#include "pic_buffer.h"
#include "inttypes.h"

#include "inttypes.h"
#include <string.h>
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"

#include <string.h>
#include "hw_settings.h"


static buffer_control_t* buff_ctrl_ptr;

static const char* TAG = "buff_ctrl";

/* internal Buffer for JPEG */
static uint8_t eth_rx_buffer_1[ IMAGE_JPEG_SIZE_BYTES ];
static uint8_t eth_rx_buffer_2[ IMAGE_JPEG_SIZE_BYTES ];


static StaticSemaphore_t xMutexBuffer;
static SemaphoreHandle_t xSemaphore = NULL;

void buff_ctrl_copy_mem_protected( void* dst_ptr, const void* src_ptr, uint32_t size )
{
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    memcpy( dst_ptr, src_ptr, size );
    xSemaphoreGive( xSemaphore );
}

void buff_ctrl_init( buffer_control_t* buff_ctrl, buff_status_t* status )
{
    if ( buff_ctrl == NULL ) return;
    buff_ctrl_ptr = buff_ctrl;
    if ( status ) buff_ctrl_ptr->status = status;

    xSemaphore = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    xSemaphoreGive( xSemaphore );

    buff_ctrl->rx_buffer_1.buff_start_ptr  = &eth_rx_buffer_1[ 0 ];
    buff_ctrl->rx_buffer_1.buff_total_size = sizeof( eth_rx_buffer_1 );
    buff_ctrl->rx_buffer_1.valid           = 0;
    buff_ctrl->rx_buffer_2.buff_start_ptr  = &eth_rx_buffer_2[ 0 ];
    buff_ctrl->rx_buffer_2.buff_total_size = sizeof( eth_rx_buffer_2 );
    buff_ctrl->rx_buffer_2.valid           = 0;
}


static void buff_ctrl_toggle_buff()
{
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_HTTP_BUFF_1_JPEG_BUFF_2 )
    {
        buff_ctrl_ptr->buff_state = BUFF_CTRL_HTTP_BUFF_2_JPEG_BUFF_1;
    }
    else buff_ctrl_ptr->buff_state = BUFF_CTRL_HTTP_BUFF_1_JPEG_BUFF_2;
}

eth_rx_buffer_t* buff_ctrl_get_jpeg_src()
{
    /* Ptr for jpeg-data to compress. returns NULL when http could not write the buffer */
    eth_rx_buffer_t* ptr = NULL;
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_HTTP_BUFF_1_JPEG_BUFF_2 )
    {
        if ( buff_ctrl_ptr->rx_buffer_2.valid ) ptr = &buff_ctrl_ptr->rx_buffer_2;
    }
    else
    {
        if ( buff_ctrl_ptr->rx_buffer_1.valid ) ptr = &buff_ctrl_ptr->rx_buffer_1;
    }

    xSemaphoreGive( xSemaphore );
    return ptr;
}


eth_rx_buffer_t* buff_ctrl_get_eth_buff()
{
    /* return reveice buffer for http task -> set valid to 0 -> http task will set to 1 if transfer successful */
    eth_rx_buffer_t* ptr = NULL;
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    buff_ctrl_toggle_buff();
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_HTTP_BUFF_1_JPEG_BUFF_2 )
    {
        ptr                              = &buff_ctrl_ptr->rx_buffer_1;
        buff_ctrl_ptr->rx_buffer_1.valid = 0;
    }

    else
    {
        ptr                              = &buff_ctrl_ptr->rx_buffer_2;
        buff_ctrl_ptr->rx_buffer_2.valid = 0;
    }
    xSemaphoreGive( xSemaphore );
    return ptr;
}

void buff_ctrl_set_eth_buff_done( uint32_t data_received )
{
    /* Marked valid by http task if jpeg received */
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_HTTP_BUFF_1_JPEG_BUFF_2 )
    {
        buff_ctrl_ptr->rx_buffer_1.data_size = data_received;
        buff_ctrl_ptr->rx_buffer_1.valid     = data_received ? 1 : 0;
    }
    else
    {
        buff_ctrl_ptr->rx_buffer_2.data_size = data_received;
        buff_ctrl_ptr->rx_buffer_2.valid     = data_received ? 1 : 0;
    }
    xSemaphoreGive( xSemaphore );
}

