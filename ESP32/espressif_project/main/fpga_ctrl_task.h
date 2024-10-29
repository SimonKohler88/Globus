/*
 * fpga_ctrl_task.h
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#ifndef MAIN_FPGA_CTRL_TASK_H_
#define MAIN_FPGA_CTRL_TASK_H_

#include "stdint.h"

/* Reflects the status of fpga status (fpga registers). not for internal control status */
struct {
	uint32_t leds;
	uint32_t width;
	uint32_t height;
	uint32_t brightness;
} typedef fpga_status_t;


void fpga_ctrl_init( fpga_status_t* status );
void fpga_ctrl_task( void *pvParameters );

#endif /* MAIN_FPGA_CTRL_TASK_H_ */
