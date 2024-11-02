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
#include "status_control_task_helper.h"
#include "fpga_ctrl_task.h"
#include "qspi.h"
#include "rotor_encoding.h"


//TODO: make all
// init frame request, reserve etc



static command_control_task_t* status = NULL;

#define STAT_CTRL_TAG "status_control_task"

//void IRAM_ATTR frame_request_isr_cb( void *arg );

StaticQueue_t xQueueBuffer_command_queue;
uint8_t command_queue_storage[ STAT_CTRL_QUEUE_NUMBER_OF_COMMANDS * sizeof( status_control_command_t ) ];


void IRAM_ATTR frame_request_isr_cb( void* arg )
{
	command_control_task_t* internal_status_ptr = ( command_control_task_t* ) arg;
	 
	status_control_command_t command = {
		.command = COMMAND_DEBUG,
		.value = 1234
	};
	
	xQueueSendFromISR( internal_status_ptr->command_queue_handle, ( void* )&command, 0 );
}


void status_control_init( status_control_status_t * status_ptr, command_control_task_t* internal_status_ptr )
{
	ESP_LOGI( STAT_CTRL_TAG, "Initializing status control..." );
	internal_status_ptr->status = status_ptr;
	status = internal_status_ptr;

	/* FPGA Control Lanes */
//	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_FRAME_REQUEST,  GPIO_MODE_INPUT  ) );
//	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_ENABLE_OUTPUT,  GPIO_MODE_OUTPUT ) );
//	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_2    ,  GPIO_MODE_INPUT  ) );
//	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_3    ,  GPIO_MODE_INPUT  ) );
//	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESET_FPGA   ,  GPIO_MODE_OUTPUT ) );
//	
//	/* Dvelopment Pins */
//	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_DEV_1        ,  GPIO_MODE_INPUT  ) );
//	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_DEV_2        ,  GPIO_MODE_INPUT  ) );
//	
//	/* install Interrupt cb for frame request */
//	ESP_ERROR_CHECK( gpio_set_intr_type( STAT_CTRL_PIN_FRAME_REQUEST, GPIO_INTR_POSEDGE ) );
//	ESP_ERROR_CHECK( gpio_intr_enable( STAT_CTRL_PIN_FRAME_REQUEST ) );
//	ESP_ERROR_CHECK( gpio_install_isr_service( ESP_INTR_FLAG_IRAM ) );
//	
//	//gpio_isr_t isr_ptr;
//	ESP_ERROR_CHECK( gpio_isr_handler_add( STAT_CTRL_PIN_FRAME_REQUEST, &frame_request_isr_cb, ( void* ) internal_status_ptr ) );
	
	/* Inbetriebsetzung */
	ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_FRAME_REQUEST, GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_ENABLE_OUTPUT, GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_2    , GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_3    , GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESET_FPGA   , GPIO_MODE_OUTPUT ) );                                                          
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_DEV_1        , GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_DEV_2        , GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_CONNECTED          , GPIO_MODE_OUTPUT ) );
	ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_0              , GPIO_MODE_OUTPUT ) );
	ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_1              , GPIO_MODE_OUTPUT ) );
	ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_2              , GPIO_MODE_OUTPUT ) );
	ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_3              , GPIO_MODE_OUTPUT ) );
	
	internal_status_ptr->command_queue_handle = xQueueCreateStatic(STAT_CTRL_QUEUE_NUMBER_OF_COMMANDS, sizeof( status_control_command_t ), &command_queue_storage[ 0 ], &xQueueBuffer_command_queue);

	init_led( internal_status_ptr );
	
}

void ibn_set_set_all_gpio_on( void )
{
	gpio_set_level(STAT_CTRL_PIN_FRAME_REQUEST, 1);
	gpio_set_level(STAT_CTRL_PIN_ENABLE_OUTPUT, 1);
	gpio_set_level(STAT_CTRL_PIN_RESERVE_2    , 1);
	gpio_set_level(STAT_CTRL_PIN_RESERVE_3    , 1);
	gpio_set_level(STAT_CTRL_PIN_RESET_FPGA   , 1);
	gpio_set_level(STAT_CTRL_PIN_DEV_1        , 1);
	gpio_set_level(STAT_CTRL_PIN_DEV_2        , 1);
	gpio_set_level(ENC_PIN_CONNECTED          , 1);
	gpio_set_level(ENC_PIN_EXP_0              , 1);
	gpio_set_level(ENC_PIN_EXP_1              , 1);
	gpio_set_level(ENC_PIN_EXP_2              , 1);
	gpio_set_level(ENC_PIN_EXP_3              , 1);
}

void ibn_set_set_all_gpio_off( void )
{
	gpio_set_level(STAT_CTRL_PIN_FRAME_REQUEST, 0);
	gpio_set_level(STAT_CTRL_PIN_ENABLE_OUTPUT, 0);
	gpio_set_level(STAT_CTRL_PIN_RESERVE_2    , 0);
	gpio_set_level(STAT_CTRL_PIN_RESERVE_3    , 0);
	gpio_set_level(STAT_CTRL_PIN_RESET_FPGA   , 0);
	gpio_set_level(STAT_CTRL_PIN_DEV_1        , 0);
	gpio_set_level(STAT_CTRL_PIN_DEV_2        , 0);
	gpio_set_level(ENC_PIN_CONNECTED          , 0);
	gpio_set_level(ENC_PIN_EXP_0              , 0);
	gpio_set_level(ENC_PIN_EXP_1              , 0);
	gpio_set_level(ENC_PIN_EXP_2              , 0);
	gpio_set_level(ENC_PIN_EXP_3              , 0);
}



void status_control_task( void * pvParameter )
{
	ESP_LOGI( STAT_CTRL_TAG, "status_control_task start" );
	if( status == NULL) 
	{
		ESP_LOGE( STAT_CTRL_TAG, "No status struct initialized" );
		return;
	}
	
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xPeriod_ms = 1000;
	
	uint8_t test_buffer[ 10 ];
	for( uint8_t i = 0; i < 10; i ++ ) test_buffer[ i ] = i;
	
	while( 1 )
	{
		//ESP_LOGI( STAT_CTRL_TAG, "status_control_cycle" );
		if( status->s_led_state ) 
		{
			ibn_set_set_all_gpio_on();
			set_led( status, 0, 10, 10 );
		}
		else
		{
			ibn_set_set_all_gpio_off();
			clear_led( status );
		}
		status->s_led_state = !status->s_led_state;
		
		fpga_ctrl_set_leds(0b11011011);
		qspi_DMA_write_debug_test( test_buffer, 10 );
		
		vTaskDelayUntil(&xLastWakeTime, xPeriod_ms);
	}
    vTaskDelete( NULL );
}























