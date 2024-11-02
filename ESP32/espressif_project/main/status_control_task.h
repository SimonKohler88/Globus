/*
 * status_control_task.h
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#ifndef MAIN_STATUS_CONTROL_TASK_H_
#define MAIN_STATUS_CONTROL_TASK_H_

#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "led_strip.h"

/* Structure for Interface */
struct {
	
	
} typedef status_control_status_t;

/* Command possibilities for task from ETH */
enum {
	COMMAND_DEBUG = 0,
	COMMAND_SEND_STATUS,
	
}; typedef uint8_t COMMANDS_t;

/* Command Structure for Queue */
struct {
	COMMANDS_t command;
	uint32_t value;
} typedef status_control_command_t;


/* Internal Structure */
struct {
	status_control_status_t* status;
	QueueHandle_t command_queue_handle;
	
	/* LED Blink */
	led_strip_handle_t led_strip;
	uint8_t s_led_state;
	
} typedef command_control_task_t;



void status_control_init( status_control_status_t * status_ptr , command_control_task_t* internal_status_ptr );
void status_control_task( void * pvParameter );


#endif /* MAIN_STATUS_CONTROL_TASK_H_ */
