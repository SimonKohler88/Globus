/*
 * PSRAM_FIFO.c
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#include "PSRAM_FIFO.h"
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

struct
{
    fifo_status_t* status;
    QueueHandle_t free_frames;
    QueueHandle_t ready_4_fpga_frames;

    fifo_frame_t current_frame_4_fpga;
    fifo_frame_t current_frame_from_rpi;
    uint8_t frame_2_fpga_in_progress;
    uint8_t frame_rpi_2_fifo_in_progress;
} typedef fifo_control_t;

static fifo_control_t fifo_control;

/* static memory allocation for Queues */
StaticQueue_t xQueueBuffer_free_frames;
uint8_t free_frames[ FIFO_NUMBER_OF_FRAMES * sizeof( fifo_frame_t ) + 1 ];
StaticQueue_t xQueueBuffer_frames_4_fpga;
uint8_t frames_4_fpga[ FIFO_NUMBER_OF_FRAMES * sizeof( fifo_frame_t ) + 1 ];

static const char* TAG = "fifo_ctrl";

fifo_frame_t static_pic_frame;
void copy_static_pic_to_PSRAM( uint8_t* start_ptr );

StaticSemaphore_t xMutexBuffer;
SemaphoreHandle_t xSemaphore = NULL;

void fifo_copy_mem_protected( void* dst_ptr, const void* src_ptr, uint32_t size )
{
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    memcpy( dst_ptr, src_ptr, size );
    xSemaphoreGive( xSemaphore );
}

void fifo_init( fifo_status_t* status )
{
    if ( status ) fifo_control.status = status;
    xSemaphore = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    xSemaphoreGive( xSemaphore );

    uint32_t frame_size_bytes = IMAGE_MAX_PIXEL_HEIGHT * IMAGE_MAX_PIXEL_WIDTH * IMAGE_BYTES_PER_PIXEL;

    // Padding space to be dividable by 4
    frame_size_bytes += frame_size_bytes % 4;

    /* Create Freertos queues. threadsafe fifo  */
    fifo_control.free_frames = xQueueCreateStatic( FIFO_NUMBER_OF_FRAMES, sizeof( fifo_frame_t ), &free_frames[ 0 ], &xQueueBuffer_free_frames );
    fifo_control.ready_4_fpga_frames =
        xQueueCreateStatic( FIFO_NUMBER_OF_FRAMES, sizeof( fifo_frame_t ), &frames_4_fpga[ 0 ], &xQueueBuffer_frames_4_fpga );

    for ( int i = 0; i < FIFO_NUMBER_OF_FRAMES; i++ )
    {
        uint8_t* frame_ptr = heap_caps_malloc( frame_size_bytes, MALLOC_CAP_SPIRAM );
        if ( frame_ptr != NULL )
        {
            fifo_frame_t frame;
            frame.frame_start_ptr = frame_ptr;
            frame.current_ptr     = frame_ptr;
            frame.total_size      = frame_size_bytes;
            frame.frame_nr        = i;
            xQueueSend( fifo_control.free_frames, &frame, 0 );
            fifo_control.status->free_frames++;
            ESP_LOGI( TAG, "Allocated Frame Buffer %" PRIu8 " , size %" PRIu32 " Bytes", i, frame_size_bytes );
        }
        else ESP_LOGE( TAG, "Failed to allocate PSRAM Memory, frame %d", i );
    }
    fifo_update_stats();

    uint8_t* frame_ptr = heap_caps_malloc( frame_size_bytes, MALLOC_CAP_SPIRAM );
    if ( frame_ptr != NULL )
    {
        static_pic_frame.frame_start_ptr = frame_ptr;
        static_pic_frame.current_ptr     = frame_ptr;
        static_pic_frame.total_size      = frame_size_bytes;
        static_pic_frame.size            = 0;
        ESP_LOGI( TAG, "Allocated Static Frame Buffer size %" PRIu32 " Bytes", frame_size_bytes );

        copy_static_pic_to_PSRAM( frame_ptr );
    }
}

uint8_t fifo_has_frame_4_fpga( void )
{
    uint8_t num = 0;
    num         = uxQueueMessagesWaiting( fifo_control.ready_4_fpga_frames );
    return num;
}

uint8_t fifo_is_frame_2_fpga_in_progress( void )
{
    uint8_t in_prog = 0;
    in_prog         = fifo_control.frame_2_fpga_in_progress;
    return in_prog;
}

fifo_frame_t* fifo_get_frame_4_fpga( void )
{
    uint8_t ret = 1;
    ESP_LOGI( "FIFO", "fifo_get_frame_for_fpga " );
    if ( fifo_control.frame_2_fpga_in_progress == 1 ) ret = 0;

    if ( ret )
    {
        const uint8_t has_frame = xQueueReceive( fifo_control.ready_4_fpga_frames, &fifo_control.current_frame_4_fpga, 0 );
        if ( has_frame == pdFALSE ) ret = 0;
        else fifo_control.frame_2_fpga_in_progress = 1;
    }

    if ( ret == 0 ) return NULL;
    return &fifo_control.current_frame_4_fpga;
}

uint8_t fifo_mark_frame_4_fpga_done( void )
{
    ESP_LOGI( "FIFO", "frame_for_fpga done" );
    /* from QSPI task */
    uint8_t success = 0;
    if ( fifo_control.frame_2_fpga_in_progress )
    {
        const uint8_t ret = xQueueSend( fifo_control.free_frames, &fifo_control.current_frame_4_fpga, 0 );
        if ( ret == pdTRUE )
        {
            fifo_control.frame_2_fpga_in_progress = 0;
            success                               = 1;
            ESP_LOGI( "FIFO", "frame_for_fpga success" );
        }
        else ESP_LOGE( "FIFO", "mark fpga frame done error" );
    }
    return success;
}

uint8_t fifo_has_free_frame( void )
{
    // ESP_LOGI( "FIFO", "has free" );
    if ( fifo_control.frame_rpi_2_fifo_in_progress ) return 0;

    uint8_t num = 0;
    num         = uxQueueMessagesWaiting( fifo_control.free_frames );
    return num;
}

uint8_t fifo_is_free_frame_in_progress( void )
{
    // ESP_LOGI( "FIFO", "free in prog" );
    return fifo_control.frame_rpi_2_fifo_in_progress;
}

fifo_frame_t* fifo_get_current_free_frame( void )
{
    if ( fifo_control.frame_rpi_2_fifo_in_progress ) return &fifo_control.current_frame_from_rpi;
    return NULL;
}

fifo_frame_t* fifo_get_free_frame( void )
{
    uint8_t ret = 1;
    if ( fifo_control.frame_rpi_2_fifo_in_progress == 1 ) ret = 0;
    if ( uxQueueMessagesWaiting( fifo_control.free_frames ) == 0 ) ret = 0;
    else
    {
        ESP_LOGI( "FIFO", "fifo_get_free_frame " );
        fifo_control.frame_rpi_2_fifo_in_progress = 1;
        const uint8_t rec = xQueueReceive( fifo_control.free_frames, &fifo_control.current_frame_from_rpi, 0 );
        if (!rec)
        {
            ESP_LOGE( "FIFO", "fifo_get_free_frame failed" );
        }
    }

    if ( ret == 0 ) return NULL;
    return &fifo_control.current_frame_from_rpi;
}

void fifo_return_free_frame( void )
{
    ESP_LOGI( "FIFO", "fifo free frame return " );
    if ( fifo_control.frame_rpi_2_fifo_in_progress )
    {
        const uint8_t ret = xQueueSend( fifo_control.free_frames, &fifo_control.current_frame_from_rpi, 0 );

        if ( ret != pdTRUE ) ESP_LOGE( "FIFO", "return frame error" );
        fifo_control.frame_rpi_2_fifo_in_progress = 0;
    }
}

uint8_t fifo_mark_free_frame_done( void )
{
    uint8_t success = 0;
    if ( fifo_control.frame_rpi_2_fifo_in_progress )
    {
        const uint8_t ret = xQueueSend( fifo_control.ready_4_fpga_frames, &fifo_control.current_frame_from_rpi, 0 );
        if ( ret == pdTRUE )
        {
            success                                   = 1;
            fifo_control.frame_rpi_2_fifo_in_progress = 0;
            ESP_LOGI( "FIFO", "fifo_mark_free_frame_done" );
        }
        else ESP_LOGE( "FIFO", "frame done error" );
    }
    return success;
}

void fifo_update_stats( void )
{
    fifo_control.status->free_frames          = uxQueueMessagesWaiting( fifo_control.free_frames );
    fifo_control.status->ready_4_fpga_frames  = uxQueueMessagesWaiting( fifo_control.ready_4_fpga_frames );
    fifo_control.status->current_frame_2_esp  = fifo_control.frame_rpi_2_fifo_in_progress;
    fifo_control.status->current_frame_2_fpga = fifo_control.frame_2_fpga_in_progress;
}

fifo_frame_t* fifo_get_static_frame( void ) { return &static_pic_frame; }

void copy_static_pic_to_PSRAM( uint8_t* start_ptr )
{
    /* Copy from code to PSRAM */
    ext_copy_static_pic_to_PSRAM( start_ptr );

    // ESP_LOGI( TAG, "Copied Data to ADDR %" PRIx32, ( uint32_t ) start_ptr );
}
