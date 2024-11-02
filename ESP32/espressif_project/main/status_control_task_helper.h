/*
 * status_control_task_helper.h
 *
 *  Created on: 30.10.2024
 *      Author: skohl
 */

#ifndef MAIN_STATUS_CONTROL_TASK_HELPER_H_
#define MAIN_STATUS_CONTROL_TASK_HELPER_H_

#include "status_control_task.h"

/* Only helper functions */

/* onboard led */
void init_led( command_control_task_t* comm_ctrl );
void set_led( command_control_task_t* comm_ctrl, uint32_t red, uint32_t green, uint32_t blue );
void clear_led( command_control_task_t* comm_ctrl );






#endif /* MAIN_STATUS_CONTROL_TASK_HELPER_H_ */
