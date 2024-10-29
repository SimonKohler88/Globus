/*
 * board_led_task.h
 *
 *  Created on: 13.10.2024
 *      Author: skohl
 */

#ifndef MAIN_MISC_TASK_H_
#define MAIN_MISC_TASK_H_


#include "hw_settings.h"

#define CONFIG_BLINK_LED_STRIP 1
#define CONFIG_BLINK_LED_STRIP_BACKEND_RMT 1


void misc_task( void *pvParameters );

#endif /* MAIN_MISC_TASK_H_ */
