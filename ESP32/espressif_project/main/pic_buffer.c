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
    if (buff_ctrl == NULL) return;
    buff_ctrl_ptr = buff_ctrl;
    if ( status ) buff_ctrl_ptr->status = status;
    xSemaphore = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    xSemaphoreGive( xSemaphore );

    uint32_t frame_size_bytes = IMAGE_TOTAL_BYTE_SIZE;

    // Padding space to be dividable by 4
    frame_size_bytes += frame_size_bytes % 4;

    uint8_t* frame_ptr = heap_caps_malloc( frame_size_bytes, MALLOC_CAP_SPIRAM );
    if ( frame_ptr != NULL )
    {

        buff_ctrl_ptr->frame_unpacked.frame_start_ptr = frame_ptr;
        buff_ctrl_ptr->frame_unpacked.current_ptr     = frame_ptr;
        buff_ctrl_ptr->frame_unpacked.total_size      = frame_size_bytes;
        buff_ctrl_ptr->frame_unpacked.size            = 0;
        ESP_LOGI( TAG, "Allocated Frame Buffer size %" PRIu32 " Bytes", frame_size_bytes );
    }
    else ESP_LOGE( TAG, "Failed to allocate PSRAM Memory for frame_unpacked" );

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

void copy_static_pic_to_PSRAM( uint8_t* start_ptr )
{
    /* Copy from code to PSRAM */
    ext_copy_static_pic_to_PSRAM( start_ptr );

    // ESP_LOGI( TAG, "Copied Data to ADDR %" PRIx32, ( uint32_t ) start_ptr );
}
