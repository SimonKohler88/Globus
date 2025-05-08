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



void init_led( led_state_t *led )
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
    ESP_ERROR_CHECK( led_strip_new_rmt_device( &strip_config, &rmt_config, &led->led_strip ) );

    /* Set all LED off to clear all pixels */
    led_strip_clear( led->led_strip );
}

void update_led( led_state_t *led )
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel( led->led_strip, 0, led->red, led->green, led->blue );
    /* Refresh the strip to send data */
    led_strip_refresh( led->led_strip );
}

void clear_led( led_state_t *led )
{
    /* Set all LED off to clear all pixels */
    led_strip_clear( led->led_strip );
}

void setup_led_color(led_state_t *led, uint8_t red, uint8_t green, uint8_t blue)
{
    led->red = red;
    led->green = green;
    led->blue = blue;
}

void set_led_red( led_state_t *led )
{
    setup_led_color( led, 128, 0, 0 );
    update_led( led );
}

void set_led_green( led_state_t *led )
{
    setup_led_color( led, 0, 128, 0 );
    update_led( led );
}

void set_led_blue( led_state_t *led )
{
    setup_led_color( led, 0, 0, 128 );
    update_led( led );
}

void set_led_cyan( led_state_t *led )
{
    setup_led_color( led, 0, 64, 64 );
    update_led( led );
}

void set_led_magenta( led_state_t *led )
{
    setup_led_color( led, 64, 0, 64 );
    update_led( led );
}

void set_led_yellow( led_state_t *led )
{
    setup_led_color( led, 64, 64, 0 );
    update_led( led );
}