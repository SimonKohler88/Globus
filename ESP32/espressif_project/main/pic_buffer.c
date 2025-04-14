/*
 * PSRAM_FIFO.c
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#include "pic_buffer.h"
#include "inttypes.h"

#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include <string.h>

#include "psram_fifo_static_pic.h"

static buffer_control_t* buff_ctrl_ptr;

static const char* TAG = "buff_ctrl";

/* internal Buffer for JPEG */
static uint8_t eth_rx_buffer_1[ IMAGE_JPEG_SIZE_BYTES ];
static uint8_t eth_rx_buffer_2[ IMAGE_JPEG_SIZE_BYTES ];

void copy_static_pic_to_PSRAM( uint8_t* start_ptr );

StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t xSemaphore = NULL;

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
    buff_ctrl->rx_buffer_2.buff_start_ptr  = &eth_rx_buffer_2[ 0 ];
    buff_ctrl->rx_buffer_2.buff_total_size = sizeof( eth_rx_buffer_2 );

    uint32_t frame_size_bytes = IMAGE_TOTAL_BYTE_SIZE;

    // Padding space to be dividable by 4
    frame_size_bytes += frame_size_bytes % 4;

    uint8_t* frame_ptr = heap_caps_malloc( frame_size_bytes, MALLOC_CAP_SPIRAM );
    if ( frame_ptr != NULL )
    {

        buff_ctrl_ptr->frame_unpacked_1.frame_start_ptr = frame_ptr;
        buff_ctrl_ptr->frame_unpacked_1.current_ptr     = frame_ptr;
        buff_ctrl_ptr->frame_unpacked_1.total_size      = frame_size_bytes;
        buff_ctrl_ptr->frame_unpacked_1.size            = 0;
        ESP_LOGI( TAG, "Allocated Frame Buffer 1 size %" PRIu32 " Bytes", frame_size_bytes );
    }
    else ESP_LOGE( TAG, "Failed to allocate PSRAM Memory for frame_unpacked 1" );

    frame_ptr = heap_caps_malloc( frame_size_bytes, MALLOC_CAP_SPIRAM );
    if ( frame_ptr != NULL )
    {

        buff_ctrl_ptr->frame_unpacked_2.frame_start_ptr = frame_ptr;
        buff_ctrl_ptr->frame_unpacked_2.current_ptr     = frame_ptr;
        buff_ctrl_ptr->frame_unpacked_2.total_size      = frame_size_bytes;
        buff_ctrl_ptr->frame_unpacked_2.size            = 0;
        ESP_LOGI( TAG, "Allocated Frame Buffer 2 size %" PRIu32 " Bytes", frame_size_bytes );
    }
    else ESP_LOGE( TAG, "Failed to allocate PSRAM Memory for frame_unpacked 2" );

    frame_ptr = heap_caps_malloc( frame_size_bytes, MALLOC_CAP_SPIRAM );
    if ( frame_ptr != NULL )
    {
        buff_ctrl_ptr->static_pic_frame.frame_start_ptr = frame_ptr;
        buff_ctrl_ptr->static_pic_frame.current_ptr     = frame_ptr;
        buff_ctrl_ptr->static_pic_frame.total_size      = frame_size_bytes;
        buff_ctrl_ptr->static_pic_frame.size            = 0;
        ESP_LOGI( TAG, "Allocated Static Frame Buffer size %" PRIu32 " Bytes", frame_size_bytes );

        copy_static_pic_to_PSRAM( frame_ptr );
    }
}

frame_unpacked_t* buff_ctrl_get_static_frame( void ) { return &buff_ctrl_ptr->static_pic_frame; }

void buff_ctrl_toggle_buff()
{
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2 )
    {
        buff_ctrl_ptr->buff_state = BUFF_CTRL_WIFI_QSI_BUFF_2_JPEG_BUFF_1;
    }
    else buff_ctrl_ptr->buff_state = BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2;
    xSemaphoreGive( xSemaphore );
}

eth_rx_buffer_t* buff_ctrl_get_jpeg_src()
{
    /* Ptr for jpeg-data to compress. returns NULL when http could not write the buffer */
    eth_rx_buffer_t* ptr = NULL;
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2 )
    {
        if ( buff_ctrl_ptr->rx_buffer_2_valid ) ptr = &buff_ctrl_ptr->rx_buffer_2;
    }

    else
    {
        if ( buff_ctrl_ptr->rx_buffer_1_valid ) ptr = &buff_ctrl_ptr->rx_buffer_1;
    }

    xSemaphoreGive( xSemaphore );
    return ptr;
}

frame_unpacked_t* buff_ctrl_get_jpeg_dst()
{
    /* Pointer to destination of jpeg decompression*/
    frame_unpacked_t* ptr = NULL;
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2 )
    {
        ptr = &buff_ctrl_ptr->frame_unpacked_2;
        buff_ctrl_ptr->frame_unpacked_2_valid = 0;
    }
    else
    {
        if ( buff_ctrl_ptr->frame_unpacked_1_valid ) ptr = &buff_ctrl_ptr->frame_unpacked_1;
        buff_ctrl_ptr->frame_unpacked_1_valid = 0;
    }
    xSemaphoreGive( xSemaphore );
    return ptr;
}
void buff_ctrl_set_jpec_dst_done( uint8_t valid )
{
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2 ) buff_ctrl_ptr->frame_unpacked_2_valid = !!valid;
    else buff_ctrl_ptr->frame_unpacked_1_valid = !!valid;
    xSemaphoreGive( xSemaphore );
}


eth_rx_buffer_t* buff_ctrl_get_eth_buff()
{
    /* return reveice buffer for http task -> set valid to 0 -> http task will set to 1 if transfer successful */
    eth_rx_buffer_t* ptr = NULL;
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2 )
    {
        ptr = &buff_ctrl_ptr->rx_buffer_1;
        buff_ctrl_ptr->rx_buffer_1_valid = 0;
    }

    else
    {
        ptr = &buff_ctrl_ptr->rx_buffer_2;
        buff_ctrl_ptr->rx_buffer_2_valid = 0;
    }
    xSemaphoreGive( xSemaphore );
    return ptr;
}

void buff_ctrl_set_eth_buff_done( uint8_t valid )
{
    /* Marked valid by http task if jpeg received */
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2 ) buff_ctrl_ptr->rx_buffer_1_valid = !!valid;
    else buff_ctrl_ptr->rx_buffer_2_valid = !!valid;
    xSemaphoreGive( xSemaphore );
}

frame_unpacked_t* buff_ctrl_get_qspi_src()
{
    /* Frame Ptr for QSPI. dont send anything if not valid */
    frame_unpacked_t* ptr = NULL;
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( buff_ctrl_ptr->buff_state == BUFF_CTRL_WIFI_QSI_BUFF_1_JPEG_BUFF_2 )
    {
        if ( buff_ctrl_ptr->frame_unpacked_1_valid ) ptr = &buff_ctrl_ptr->frame_unpacked_1;
    }
    else
    {
        if ( buff_ctrl_ptr->frame_unpacked_2_valid ) ptr = &buff_ctrl_ptr->frame_unpacked_2;
    }
    xSemaphoreGive( xSemaphore );
    return ptr;
}

void copy_static_pic_to_PSRAM( uint8_t* start_ptr )
{
    /* Copy from code to PSRAM */
    ext_copy_static_pic_to_PSRAM( start_ptr );

    // ESP_LOGI( TAG, "Copied Data to ADDR %" PRIx32, ( uint32_t ) start_ptr );
}
