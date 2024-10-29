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


struct {
	
	
} typedef status_control_status_t;


struct {
	uint8_t command_num;
	uint32_t value;	
} typedef status_control_command_t;



void status_control_init( status_control_status_t * status_ptr );
void status_control_task( void * pvParameter );


#endif /* MAIN_STATUS_CONTROL_TASK_H_ */
