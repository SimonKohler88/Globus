/*
 * status_control_task.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "status_control_task.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "hw_settings.h"

//TODO: make all
// init frame request, reserve etc



status_control_status_t * status = NULL;
#define STAT_CTRL_TAG "status_control_task"

//void IRAM_ATTR frame_request_isr_cb( void );

StaticQueue_t xQueueBuffer_frames_4_fpga;
uint8_t frames_4_fpga[ STAT_CTRL_QUEUE_NUMBER_OF_COMMANDS * sizeof( status_control_command_t ) ];

void status_control_init( status_control_status_t * status_ptr )
{
	status = status_ptr;
/*
	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_FRAME_REQUEST,  GPIO_MODE_INPUT  ) );
	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_ENABLE_OUTPUT,  GPIO_MODE_OUTPUT ) );
	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_2    ,  GPIO_MODE_INPUT  ) );
	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_3    ,  GPIO_MODE_INPUT  ) );
	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESET_FPGA   ,  GPIO_MODE_OUTPUT ) );
	*/
	
	/* install Interrupt cb for frame request */
	//ESP_ERROR_CHECK( gpio_set_intr_type( STAT_CTRL_PIN_FRAME_REQUEST, GPIO_INTR_POSEDGE ) );
	//ESP_ERROR_CHECK( gpio_intr_enable( STAT_CTRL_PIN_FRAME_REQUEST ) );
	//ESP_ERROR_CHECK( gpio_install_isr_service( ESP_INTR_FLAG_IRAM ) );
	
	//gpio_isr_t isr_ptr;
	//ESP_ERROR_CHECK( gpio_isr_handler_add( STAT_CTRL_PIN_FRAME_REQUEST, frame_request_isr_cb, NULL ) );

}

//void IRAM_ATTR frame_request_isr_cb( void )
//{
	
//}


void status_control_task( void * pvParameter )
{
	if( status == NULL) ESP_LOGE( STAT_CTRL_TAG, "No status struct initialized" );
}
