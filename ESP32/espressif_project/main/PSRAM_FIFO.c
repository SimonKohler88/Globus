/*
 * PSRAM_FIFO.c
 *
 *  Created on: 19.10.2024
 *      Author: skohl
 */

#include "inttypes.h"
#include "PSRAM_FIFO.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/task.h"





struct {
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


void fifo_init( fifo_status_t* status )
{
	
	if( status ) fifo_control.status= status;
	
	uint32_t frame_size_bytes = IMAGE_MAX_PIXEL_HEIGHT * IMAGE_MAX_PIXEL_WIDTH * IMAGE_BYTES_PER_PIXEL;
	
	/* Create Freertos queues. threadsafe fifo  */
	fifo_control.free_frames = xQueueCreateStatic( FIFO_NUMBER_OF_FRAMES, sizeof( fifo_frame_t ), &free_frames[ 0 ], &xQueueBuffer_free_frames );
	fifo_control.ready_4_fpga_frames = xQueueCreateStatic( FIFO_NUMBER_OF_FRAMES, sizeof( fifo_frame_t ), &frames_4_fpga[ 0 ], &xQueueBuffer_frames_4_fpga );
	
	for( int i = 0; i < FIFO_NUMBER_OF_FRAMES; i++ )
	{
		uint8_t* frame_ptr = heap_caps_malloc(frame_size_bytes, MALLOC_CAP_SPIRAM);
		if( frame_ptr != NULL )
		{
			fifo_frame_t frame;
			frame.frame_start_ptr = frame_ptr;
			frame.current_ptr = frame_ptr;
			xQueueSend( fifo_control.free_frames, (void*)&frame, 0 );
			fifo_control.status->free_frames ++;
			ESP_LOGI(TAG, "Allocated Frame Buffer %"PRIu8" , size %"PRIu32" Bytes", i, frame_size_bytes);
		}
		else ESP_LOGE(TAG, "Failed to allocate PSRAM Memory, frame %d", i);
	}
}

uint8_t fifo_has_frame_4_fpga( void )
{
	if( uxQueueMessagesWaiting( fifo_control.ready_4_fpga_frames ) > 0 ) return 1;
	return 0;
}

uint8_t fifo_get_frame_4_fpga(fifo_frame_t* frame_info)
{
	if( fifo_control.frame_2_fpga_in_progress == 1 ) return 0;
	
	uint8_t has_frame = xQueueReceive( fifo_control.ready_4_fpga_frames, &fifo_control.current_frame_4_fpga, 0 );
	if( has_frame == pdFALSE ) return 0;
	
	*frame_info = fifo_control.current_frame_4_fpga;
	fifo_control.frame_2_fpga_in_progress = 1;
	
	if( fifo_control.status->ready_4_fpga_frames > 0 ) fifo_control.status->ready_4_fpga_frames --;
	return 1;
}
/*
fifo_control.status->free_frames = uxQueueMessagesWaiting( fifo_control.free_frames );
	fifo_control.status->ready_4_fpga_frames = 
*/
void fifo_mark_frame_4_fpga_done( void )
{
	/* from DMA -> QSPI transfer finished. most likely called from ISR */
	if( fifo_control.frame_2_fpga_in_progress )
	{
		fifo_control.current_frame_4_fpga.current_ptr = fifo_control.current_frame_4_fpga.frame_start_ptr;
		fifo_control.current_frame_4_fpga.size = 0; 
		xQueueSendFromISR( fifo_control.free_frames, ( void* )&fifo_control.current_frame_4_fpga, 0 );
		fifo_control.frame_2_fpga_in_progress = 0;
		
		fifo_control.status->free_frames ++;
	}
}

uint8_t fifo_has_free_frame( void )
{
	if( uxQueueMessagesWaiting( fifo_control.free_frames ) > 0 ) return 1;
	return 0;
}

uint8_t fifo_get_free_frame( fifo_frame_t* frame_info )
{
	if( fifo_control.frame_rpi_2_fifo_in_progress == 1 ) return 0;
	
	uint8_t has_frame = xQueueReceive( fifo_control.free_frames, &fifo_control.current_frame_from_rpi, 0 );
	if( has_frame == pdFALSE ) return 0;
	
	*frame_info = fifo_control.current_frame_from_rpi;
	fifo_control.frame_rpi_2_fifo_in_progress = 1; 
	
	if( fifo_control.status->free_frames ) fifo_control.status->free_frames --;
	
	return 1; 
}

void fifo_mark_free_frame_done( void )
{
	/* from DMA -> TFTP transfer finished. most likely called from ISR */
	if( fifo_control.frame_rpi_2_fifo_in_progress )
	{
		xQueueSendFromISR(fifo_control.ready_4_fpga_frames, ( void * )&fifo_control.current_frame_from_rpi, 0);
		fifo_control.frame_2_fpga_in_progress = 0;
		
		fifo_control.status->ready_4_fpga_frames ++;
	}
}

void fifo_update_stats( void )
{
	fifo_control.status->free_frames = uxQueueMessagesWaiting( fifo_control.free_frames );
	fifo_control.status->ready_4_fpga_frames = uxQueueMessagesWaiting( fifo_control.free_frames );
}

