/*
 * status_control_task.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "status_control_task.h"

#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "hw_settings.h"
#include "qspi.h"
#include "status_control_task_helper.h"
#include "wifi.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

static command_control_task_t* status = NULL;

#define STAT_CTRL_TAG    "status_control_task"

void status_control_init( status_control_status_t* status_ptr, command_control_task_t* internal_status_ptr, task_handles_t* task_handles )
{
    ESP_LOGI( STAT_CTRL_TAG, "Initializing status control..." );
    internal_status_ptr->status = status_ptr;
    status                      = internal_status_ptr;
    status->task_handles        = task_handles;

    gpio_config_t config = {
        .intr_type = GPIO_INTR_POSEDGE, .mode = GPIO_MODE_INPUT, .pull_up_en = 1, .pin_bit_mask = 1 << STAT_CTRL_PIN_FRAME_REQUEST };

    ESP_ERROR_CHECK( gpio_config( &config ) );

    /* GPIO Directions */
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_1, GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_2, GPIO_MODE_INPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_3, GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESET_FPGA, GPIO_MODE_OUTPUT ) );

    ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_CONNECTED, GPIO_MODE_INPUT ) );

    gpio_set_level( STAT_CTRL_PIN_RESET_FPGA, 1 );

    /* init led */
    init_led( internal_status_ptr );
}

void ibn_set_set_all_gpio_on( void )
{
    gpio_set_level( STAT_CTRL_PIN_RESERVE_1, 1 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_2, 1 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_3, 1 );
    gpio_set_level( STAT_CTRL_PIN_RESET_FPGA, 1 );
    gpio_set_level( ENC_PIN_CONNECTED, 1 );
}

void ibn_set_set_all_gpio_off( void )
{
    gpio_set_level( STAT_CTRL_PIN_RESERVE_1, 0 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_2, 0 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_3, 0 );
    gpio_set_level( STAT_CTRL_PIN_RESET_FPGA, 0 );
}

void set_gpio_reserve_1_async( uint8_t value ) { gpio_set_level( STAT_CTRL_PIN_RESERVE_1, !!value ); }

void status_control_task( void* pvParameter )
{
    ESP_LOGI( STAT_CTRL_TAG, "status_control_task start" );
    if ( status == NULL )
    {
        ESP_LOGE( STAT_CTRL_TAG, "No status struct initialized" );
        return;
    }

    /* for toggling led */
    uint32_t led_color              = 0x01020020;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;

    /* wait until all tasks have a handle */
    while ( status->task_handles->http_task_handle == NULL ||
             status->task_handles->WIFI_task_handle == NULL )
    {
        vTaskDelay( 10 );
    }
    xTaskNotifyIndexed( status->task_handles->WIFI_task_handle, TASK_NOTIFY_WIFI_START_BIT, 0, eSetBits );
    xTaskNotifyWaitIndexed( TASK_NOTIFY_CTRL_WIFI_FINISHED_BIT, pdFALSE, ULONG_MAX, NULL, portMAX_DELAY );
    xTaskNotifyIndexed( status->task_handles->http_task_handle, TASK_NOTIFY_WIFI_READY_BIT, 0, eSetBits );


    if ( CTRL_TASK_VERBOSE ) ESP_LOGI( STAT_CTRL_TAG, "Enter Loop" );
    xLastWakeTime = xTaskGetTickCount ();
    while ( 1 )
    {
        if ( STAT_CTRL_ENABLE_LED )
        {
            uint8_t temp = ( led_color >> 31 );
            led_color    = led_color << 1;
            led_color |= temp;
            set_led( status, ( led_color >> 16 ), ( led_color >> 8 ), led_color );
        }

        xTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
    vTaskDelete( NULL );
}


