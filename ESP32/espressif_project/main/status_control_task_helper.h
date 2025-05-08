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
 * Initializes the LED strip for a given led_state_t.
 *
 * This function sets up the LED strip by configuring the GPIO and pixel count,
 * establishing the RMT (Remote Control) configuration with a 10MHz resolution,
 * and linking it to the led_state_t structure provided.
 * It clears the LEDs to ensure all pixels are turned off initially.
 *
 * @param led A pointer to the led_state_t structure that will
 *                  be associated with the LED strip configuration.
 */
void init_led( led_state_t* led );

/**
 * @brief Sets the LED color using RGB values.
 *
 * This function configures the LED color by setting the red, green, and blue
 * intensity levels. The values for each color component range from 0 (0%) to 255 (100%).
 * Once the LED color is set, the function refreshes the LED strip to apply the changes.
 *
 * @param led Pointer to a led_state_t structure containing the LED strip configuration.
 */
void update_led( led_state_t* led );

/**
 * Clears all the LEDs by setting them off in the LED strip associated with the given led_state_t structure.
 *
 * @param led A pointer to the led_state_t structure which contains information about the LED strip to be cleared.
 */
void clear_led( led_state_t *led );

void setup_led_color(led_state_t *led, uint8_t red, uint8_t green, uint8_t blue);

#endif /* MAIN_STATUS_CONTROL_TASK_HELPER_H_ */
