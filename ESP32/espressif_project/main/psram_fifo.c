/*
 * PSRAM_FIFO.c
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#include "psram_fifo.h"

#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "inttypes.h"
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
uint8_t free_frames[ FIFO_NUMBER_OF_FRAMES * sizeof( fifo_frame_t ) ];
StaticQueue_t xQueueBuffer_frames_4_fpga;
uint8_t frames_4_fpga[ FIFO_NUMBER_OF_FRAMES * sizeof( fifo_frame_t ) ];

static const char* TAG = "fifo_ctrl";

static fifo_frame_t static_pic_frame;
void copy_static_pic_to_PSRAM( uint8_t* start_ptr );

static StaticSemaphore_t xMutexBuffer;
static SemaphoreHandle_t xSemaphore = NULL;

void fifo_copy_mem_protected( void* dst_ptr, const void* src_ptr, uint32_t size )
{
    if (dst_ptr == NULL || src_ptr == NULL || size == 0) return;
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    memcpy( dst_ptr, src_ptr, size );
    xSemaphoreGive( xSemaphore );
}

void fifo_semaphore_take( void ) { xSemaphoreTake( xSemaphore, portMAX_DELAY ); }
void fifo_semaphore_give( void ) { xSemaphoreGive( xSemaphore ); }

void fifo_init( fifo_status_t* status )
{
    if ( status ) fifo_control.status = status;
    xSemaphore = xSemaphoreCreateMutexStatic( &xMutexBuffer );
    xSemaphoreGive( xSemaphore );

    uint32_t frame_size_bytes = IMAGE_TOTAL_BYTE_SIZE;

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
            frame.size            = 0;
            xQueueSend( fifo_control.free_frames, &frame, 0 );
            fifo_control.status->free_frames++;
            ESP_LOGI( TAG, "Allocated Frame Buffer %" PRIu8 " , size %" PRIu32 " Bytes at 0x%" PRIx32, i, frame_size_bytes,
                      ( uint32_t ) frame.frame_start_ptr );
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
    if ( FIFO_VERBOSE ) ESP_LOGI( "FIFO", "fifo_get_frame_for_fpga " );

    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( fifo_control.frame_2_fpga_in_progress == 1 ) ret = 0;

    if ( ret )
    {
        uint8_t has_frame = xQueueReceive( fifo_control.ready_4_fpga_frames, &fifo_control.current_frame_4_fpga, 0 );
        if ( has_frame == pdFALSE ) ret = 0;
        else fifo_control.frame_2_fpga_in_progress = 1;
    }
    xSemaphoreGive( xSemaphore );

    if ( ret == 0 ) return NULL;
    return &fifo_control.current_frame_4_fpga;
}

void fifo_mark_frame_4_fpga_done( void )
{
    if ( FIFO_VERBOSE ) ESP_LOGI( "FIFO", "frame_for_fpga done" );
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    /* from QSPI task */
    // if ( fifo_control.frame_2_fpga_in_progress )
    // {
    //     fifo_control.current_frame_4_fpga.current_ptr = fifo_control.current_frame_4_fpga.frame_start_ptr;
    //     fifo_control.current_frame_4_fpga.size        = 0;
    //     uint8_t ret                                   = xQueueSend( fifo_control.free_frames, &fifo_control.current_frame_4_fpga, 0 );
    //     if ( ret == pdTRUE ) fifo_control.frame_2_fpga_in_progress = 0;
    //     else ESP_LOGE( "FIFO", "mark fpga frame done error" );
    // }

    fifo_control.current_frame_4_fpga.current_ptr = fifo_control.current_frame_4_fpga.frame_start_ptr;
    fifo_control.current_frame_4_fpga.size        = 0;
    xQueueSend( fifo_control.free_frames, &fifo_control.current_frame_4_fpga, 0 );
    fifo_control.frame_2_fpga_in_progress = 0;

    xSemaphoreGive( xSemaphore );
}

uint8_t fifo_has_free_frame( void )
{
    if ( FIFO_VERBOSE ) ESP_LOGI( "FIFO", "has free" );
    if ( fifo_control.frame_rpi_2_fifo_in_progress ) return 0;

    uint8_t num = 0;
    num         = uxQueueMessagesWaiting( fifo_control.free_frames );
    return num;
}

uint8_t fifo_is_free_frame_in_progress( void )
{
    if ( FIFO_VERBOSE ) ESP_LOGI( "FIFO", "free in prog" );
    uint8_t in_prog = 0;
    in_prog         = fifo_control.frame_rpi_2_fifo_in_progress;
    return in_prog;
}

fifo_frame_t* fifo_get_current_free_frame( void )
{
    if ( fifo_control.frame_rpi_2_fifo_in_progress ) return &fifo_control.current_frame_from_rpi;
    return NULL;
}

fifo_frame_t* fifo_get_free_frame( void )
{
    uint8_t ret = 1;

    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    if ( fifo_control.frame_rpi_2_fifo_in_progress == 1 ) ret = 0;
    if ( uxQueueMessagesWaiting( fifo_control.free_frames ) == 0 ) ret = 0;
    else
    {
        if ( FIFO_VERBOSE ) ESP_LOGI( "FIFO", "fifo_get_free_frame " );
        fifo_control.frame_rpi_2_fifo_in_progress = 1;
        xQueueReceive( fifo_control.free_frames, &fifo_control.current_frame_from_rpi, 0 );
    }
    xSemaphoreGive( xSemaphore );

    if ( ret == 0 ) return NULL;
    return &fifo_control.current_frame_from_rpi;
}

void fifo_return_free_frame( void )
{
    if ( FIFO_VERBOSE ) ESP_LOGI( "FIFO", "fifo free frame return " );
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    // if ( fifo_control.frame_rpi_2_fifo_in_progress )
    // {
    //     uint8_t ret = xQueueSend( fifo_control.free_frames, &fifo_control.current_frame_from_rpi, 0 );
    //
    //     if ( ret != pdTRUE ) ESP_LOGE( "FIFO", "return frame error" );
    //     fifo_control.frame_rpi_2_fifo_in_progress = 0;
    // }
    uint8_t ret = xQueueSend( fifo_control.free_frames, &fifo_control.current_frame_from_rpi, 0 );
    if ( ret != pdTRUE ) ESP_LOGE( "FIFO", "return frame error" );
    fifo_control.frame_rpi_2_fifo_in_progress = 0;
    xSemaphoreGive( xSemaphore );
}

void fifo_mark_free_frame_done( void )
{
    if ( FIFO_VERBOSE ) ESP_LOGI( "FIFO", "fifo_mark_free_frame_done" );
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
    // if ( fifo_control.frame_rpi_2_fifo_in_progress )
    // {
    //     uint8_t ret = xQueueSend( fifo_control.ready_4_fpga_frames, &fifo_control.current_frame_from_rpi, 0 );
    //     if ( ret == pdTRUE ) fifo_control.frame_rpi_2_fifo_in_progress = 0;
    //     else ESP_LOGE( "FIFO", "frame done error" );
    // }

    uint8_t ret = xQueueSend( fifo_control.ready_4_fpga_frames, &fifo_control.current_frame_from_rpi, 0 );
    if ( ret != pdTRUE ) ESP_LOGE( "FIFO", "frame done error" );
    fifo_control.frame_rpi_2_fifo_in_progress = 0;
    xSemaphoreGive( xSemaphore );
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
}
