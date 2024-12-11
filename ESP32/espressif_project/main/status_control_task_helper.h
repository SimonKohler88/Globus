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

/**
 * Initializes the LED strip for a given command control task.
 *
 * This function sets up the LED strip by configuring the GPIO and pixel count,
 * establishing the RMT (Remote Control) configuration with a 10MHz resolution,
 * and linking it to the command control task structure provided.
 * It clears the LEDs to ensure all pixels are turned off initially.
 *
 * @param comm_ctrl A pointer to the command control task structure that will
 *                  be associated with the LED strip configuration.
 */
void init_led( command_control_task_t* comm_ctrl );

/**
 * @brief Sets the LED color using RGB values.
 *
 * This function configures the LED color by setting the red, green, and blue
 * intensity levels. The values for each color component range from 0 (0%) to 255 (100%).
 * Once the LED color is set, the function refreshes the LED strip to apply the changes.
 *
 * @param comm_ctrl Pointer to a command control task structure containing the LED strip configuration.
 * @param red Intensity of the red color component (0 to 255).
 * @param green Intensity of the green color component (0 to 255).
 * @param blue Intensity of the blue color component (0 to 255).
 */
void set_led( command_control_task_t* comm_ctrl, uint32_t red, uint32_t green, uint32_t blue );

/**
 * Clears all the LEDs by setting them off in the LED strip associated with the given command control task.
 *
 * @param comm_ctrl A pointer to the command control task structure which contains information about the LED strip to be cleared.
 */
void clear_led( command_control_task_t* comm_ctrl );






#endif /* MAIN_STATUS_CONTROL_TASK_HELPER_H_ */
