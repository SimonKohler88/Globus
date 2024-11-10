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
	uint32_t current_speed;
} typedef fpga_status_t;

//struct {
//	uint32_t leds;
//	uint32_t width;
//	uint32_t height;
//	uint32_t brightness;
//} typedef fpga_parameter_t;

struct {
	uint32_t missed_spi_command_writes;
	uint32_t missed_spi_status_reads;
} typedef fpga_task_status_t;

void fpga_ctrl_init( fpga_status_t* status, fpga_task_status_t* fpga_task_status_ptr );
void fpga_ctrl_task( void *pvParameters );
uint8_t fpga_ctrl_set_leds( uint8_t leds );

#endif /* MAIN_FPGA_CTRL_TASK_H_ */
