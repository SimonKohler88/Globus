/*
 * status_control_task_helper.c
 *
 *  Created on: 30.10.2024
 *      Author: skohl
 */

#include "status_control_task_helper.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "hw_settings.h"
#include "led_strip.h"



void init_led( command_control_task_t *comm_ctrl )
{
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_BLINK_GPIO,
        .max_leds       = 1,  // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz  = 10 * 1000 * 1000,  // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK( led_strip_new_rmt_device( &strip_config, &rmt_config, &comm_ctrl->led_strip ) );

    /* Set all LED off to clear all pixels */
    led_strip_clear( comm_ctrl->led_strip );
}

void set_led( command_control_task_t *comm_ctrl, uint32_t red, uint32_t green, uint32_t blue )
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel( comm_ctrl->led_strip, 0, red, green, blue );
    /* Refresh the strip to send data */
    led_strip_refresh( comm_ctrl->led_strip );
}

void clear_led( command_control_task_t *comm_ctrl )
{
    /* Set all LED off to clear all pixels */
    led_strip_clear( comm_ctrl->led_strip );
}
