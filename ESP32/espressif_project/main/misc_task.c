/*
 * board_led_task.c
 *
 *  Created on: 13.10.2024
 *      Author: skohl
 */


#include "misc_task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"


static const char *TAG = "misc_task";


static led_strip_handle_t led_strip;
static uint8_t s_led_state = 0;


#define BLINK_GPIO CONFIG_BLINK_GPIO

static void blink_led( void )
{
    /* If the addressable LED is enabled */
    if ( s_led_state ) 
    {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel( led_strip, 0, LED_COLOR_R , LED_COLOR_G, LED_COLOR_B );
        /* Refresh the strip to send data */
        led_strip_refresh( led_strip );
    } 
    else 
    {
        /* Set all LED off to clear all pixels */
        led_strip_clear( led_strip );
    }
}

static void configure_led( void )
{
    ESP_LOGI( TAG, "Example configured to blink addressable LED!" );
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK( led_strip_new_rmt_device( &strip_config, &rmt_config, &led_strip ) );
	
    /* Set all LED off to clear all pixels */
    led_strip_clear( led_strip );
}

void misc_task( void *pvParameters )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xPeriod_ms = 500;
	
	configure_led();
	
	 while (1) {
		//ESP_LOGI(TAG, "Blink the LED ");
		 blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
		 
		vTaskDelayUntil(&xLastWakeTime, xPeriod_ms);
	}
    vTaskDelete( NULL );
}
