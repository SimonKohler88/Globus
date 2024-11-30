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

struct
{
    fifo_status_t *status;
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

static const char *TAG = "fifo_ctrl";

void fifo_init( fifo_status_t *status )
{

    if ( status ) fifo_control.status = status;

    uint32_t frame_size_bytes =
        IMAGE_MAX_PIXEL_HEIGHT * IMAGE_MAX_PIXEL_WIDTH * IMAGE_BYTES_PER_PIXEL;

    // Padding space to be dividable by 4
    frame_size_bytes += frame_size_bytes % 4;

    /* Create Freertos queues. threadsafe fifo  */
    fifo_control.free_frames =
        xQueueCreateStatic( FIFO_NUMBER_OF_FRAMES, sizeof( fifo_frame_t ),
                            &free_frames[ 0 ], &xQueueBuffer_free_frames );
    fifo_control.ready_4_fpga_frames =
        xQueueCreateStatic( FIFO_NUMBER_OF_FRAMES, sizeof( fifo_frame_t ),
                            &frames_4_fpga[ 0 ], &xQueueBuffer_frames_4_fpga );

    for ( int i = 0; i < FIFO_NUMBER_OF_FRAMES; i++ )
    {
        uint8_t *frame_ptr =
            heap_caps_malloc( frame_size_bytes, MALLOC_CAP_SPIRAM );
        if ( frame_ptr != NULL )
        {
            fifo_frame_t frame;
            frame.frame_start_ptr = frame_ptr;
            frame.current_ptr     = frame_ptr;
            xQueueSend( fifo_control.free_frames, ( void * )&frame, 0 );
            fifo_control.status->free_frames++;
            ESP_LOGI( TAG,
                      "Allocated Frame Buffer %" PRIu8 " , size %" PRIu32
                      " Bytes",
                      i, frame_size_bytes );
        }
        else ESP_LOGE( TAG, "Failed to allocate PSRAM Memory, frame %d", i );
    }
    fifo_update_stats();
}

uint8_t fifo_has_frame_4_fpga( void )
{
    return uxQueueMessagesWaiting( fifo_control.ready_4_fpga_frames );
}

uint8_t fifo_is_frame_2_fpga_in_progress( void )
{
    return fifo_control.frame_2_fpga_in_progress;
}

fifo_frame_t *fifo_get_frame_4_fpga( void )
{
    if ( fifo_control.frame_2_fpga_in_progress == 1 ) return NULL;
    ESP_LOGI( "FIFO", "fifo_get_frame_for_fpga " );

    uint8_t has_frame = xQueueReceive( fifo_control.ready_4_fpga_frames,
                                       &fifo_control.current_frame_4_fpga, 0 );
    if ( has_frame == pdFALSE ) return NULL;

    fifo_control.frame_2_fpga_in_progress = 1;

    fifo_update_stats();
    return &fifo_control.current_frame_4_fpga;
}

void fifo_mark_frame_4_fpga_done( void )
{
    /* from QSPI task */
    if ( fifo_control.frame_2_fpga_in_progress )
    {
        fifo_control.current_frame_4_fpga.current_ptr =
            fifo_control.current_frame_4_fpga.frame_start_ptr;
        fifo_control.current_frame_4_fpga.size = 0;
        xQueueSend( fifo_control.free_frames,
                    ( void * )&fifo_control.current_frame_4_fpga, 0 );
        fifo_control.frame_2_fpga_in_progress = 0;

        fifo_update_stats();
    }
}

uint8_t fifo_has_free_frame( void )
{
    return uxQueueMessagesWaiting( fifo_control.free_frames );
}

fifo_frame_t *fifo_get_free_frame( void )
{
    if ( fifo_control.frame_rpi_2_fifo_in_progress == 1 ) return NULL;

    uint8_t has_frame = xQueueReceive(
        fifo_control.free_frames, &fifo_control.current_frame_from_rpi, 0 );
    if ( has_frame == pdFALSE ) return NULL;

    fifo_control.frame_rpi_2_fifo_in_progress = 1;

    fifo_update_stats();

    return &fifo_control.current_frame_from_rpi;
}

void fifo_return_free_frame( void )
{
    xQueueSend( fifo_control.free_frames, &fifo_control.current_frame_from_rpi,
                0 );
    fifo_control.frame_rpi_2_fifo_in_progress = 0;
    fifo_update_stats();
}

void fifo_mark_free_frame_done( void )
{
    ESP_LOGI( "FIFO", "fifo_mark_free_frame_done %d ",
              fifo_control.frame_rpi_2_fifo_in_progress );

    if ( fifo_control.frame_rpi_2_fifo_in_progress )
    {
        xQueueSend( fifo_control.ready_4_fpga_frames,
                    ( void * )&fifo_control.current_frame_from_rpi, 0 );
        fifo_control.frame_rpi_2_fifo_in_progress = 0;

        fifo_update_stats();
    }
}

void fifo_update_stats( void )
{
    fifo_control.status->free_frames =
        uxQueueMessagesWaiting( fifo_control.free_frames );
    fifo_control.status->ready_4_fpga_frames =
        uxQueueMessagesWaiting( fifo_control.ready_4_fpga_frames );
    fifo_control.status->current_frame_2_esp =
        fifo_control.frame_rpi_2_fifo_in_progress;
    fifo_control.status->current_frame_2_fpga =
        fifo_control.frame_2_fpga_in_progress;
}
