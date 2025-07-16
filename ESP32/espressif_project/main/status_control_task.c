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
#include "psram_fifo.h"
#include "qspi.h"
#include "status_control_task_helper.h"
#include "wifi.h"

#include <esp_timer.h>

#define STAT_CTRL_TAG    "status_control_task"
#define TIMER_RESOLUTION ( 10 * 1000 )

/* internal structure instances */
static command_control_task_t* status = NULL;
gptimer_handle_t gptimer              = NULL;

volatile uint8_t line_toggle = 0;

static bool IRAM_ATTR frame_request_timer_isr_cb( gptimer_handle_t timer, const gptimer_alarm_event_data_t* edata, void* user_ctx )
{
    gpio_set_level( STAT_CTRL_PIN_RESERVE_3, line_toggle );
    line_toggle = !line_toggle;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xHigherPriorityTaskWoken            = qspi_request_frame( 1 );
    return xHigherPriorityTaskWoken;
}

void status_control_init( status_control_status_t* status_ptr, command_control_task_t* internal_status_ptr, task_handles_t* task_handles )
{
    ESP_LOGI( STAT_CTRL_TAG, "Initializing status control..." );
    internal_status_ptr->status = status_ptr;
    status                      = internal_status_ptr;
    status->task_handles        = task_handles;

    gpio_config_t config = {
        .intr_type = GPIO_INTR_POSEDGE, .mode = GPIO_MODE_INPUT, .pull_up_en = 1, .pin_bit_mask = 1 << STAT_CTRL_PIN_FRAME_REQUEST };

    ESP_ERROR_CHECK( gpio_config( &config ) );

    /* FPGA Control Lanes */

    /* GPIO Directions */
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_1, GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_2, GPIO_MODE_INPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESERVE_3, GPIO_MODE_OUTPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( STAT_CTRL_PIN_RESET_FPGA, GPIO_MODE_OUTPUT ) );

    ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_CONNECTED, GPIO_MODE_INPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_0, GPIO_MODE_INPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_1, GPIO_MODE_INPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_2, GPIO_MODE_INPUT ) );
    ESP_ERROR_CHECK( gpio_set_direction( ENC_PIN_EXP_3, GPIO_MODE_INPUT ) );

    gpio_set_level( STAT_CTRL_PIN_RESET_FPGA, 1 );

    /* init timer with callback for QSPI-thread start */
    gptimer_config_t timer_config = {
        .clk_src       = GPTIMER_CLK_SRC_DEFAULT,
        .direction     = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION,
        .intr_priority = 3,
    };
    ESP_ERROR_CHECK( gptimer_new_timer( &timer_config, &gptimer ) );

    gptimer_alarm_config_t alarm_config = {
        .reload_count               = 0,                                                                  // counter will reload with 0 on alarm event
        .alarm_count                = ( uint32_t ) ( TIMER_RESOLUTION / ( STAT_CTRL_QSPI_FRAME_RATE ) ),  // period = 1s @resolution 1MHz
        .flags.auto_reload_on_alarm = true,                                                               // enable auto-reload
    };
    ESP_ERROR_CHECK( gptimer_set_alarm_action( gptimer, &alarm_config ) );

    gptimer_event_callbacks_t cbs = {
        .on_alarm = frame_request_timer_isr_cb,  // register user callback
    };
    ESP_ERROR_CHECK( gptimer_register_event_callbacks( gptimer, &cbs, NULL ) );

#ifndef DEVELOPMENT_SET_QSPI_STATIC
    /* frame triggered from status control in static mode */
    ESP_ERROR_CHECK( gptimer_enable( gptimer ) );
#endif

    /* init led */
    init_led( &internal_status_ptr->led );
}

void ibn_set_set_all_gpio_on( void )
{
    //	gpio_set_level(STAT_CTRL_PIN_FRAME_REQUEST, 1);
    gpio_set_level( STAT_CTRL_PIN_RESERVE_1, 1 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_2, 1 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_3, 1 );
    gpio_set_level( STAT_CTRL_PIN_RESET_FPGA, 1 );
    //	gpio_set_level(STAT_CTRL_PIN_DEV_1        , 1);
    //	gpio_set_level(STAT_CTRL_PIN_DEV_2        , 1);
    gpio_set_level( ENC_PIN_CONNECTED, 1 );
    gpio_set_level( ENC_PIN_EXP_0, 1 );
    gpio_set_level( ENC_PIN_EXP_1, 1 );
    gpio_set_level( ENC_PIN_EXP_2, 1 );
    gpio_set_level( ENC_PIN_EXP_3, 1 );
}

void ibn_set_set_all_gpio_off( void )
{
    //	gpio_set_level(STAT_CTRL_PIN_FRAME_REQUEST, 0);
    gpio_set_level( STAT_CTRL_PIN_RESERVE_1, 0 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_2, 0 );
    gpio_set_level( STAT_CTRL_PIN_RESERVE_3, 0 );
    gpio_set_level( STAT_CTRL_PIN_RESET_FPGA, 0 );
    //	gpio_set_level(STAT_CTRL_PIN_DEV_1        , 0);
    //	gpio_set_level(STAT_CTRL_PIN_DEV_2        , 0);
    gpio_set_level( ENC_PIN_CONNECTED, 0 );
    gpio_set_level( ENC_PIN_EXP_0, 0 );
    gpio_set_level( ENC_PIN_EXP_1, 0 );
    gpio_set_level( ENC_PIN_EXP_2, 0 );
    gpio_set_level( ENC_PIN_EXP_3, 0 );
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
    uint32_t ulNotifyValueJPEG = 0;
    uint32_t ulNotifyValueHTTP = 0;
    uint32_t ulNotifyValueWIFI = 0;

    set_led_cyan( &status->led );

#ifdef DEVELOPMENT_SET_QSPI_STATIC
    TickType_t xLastWakeTime    = xTaskGetTickCount();
    const TickType_t xPeriod_ms = 60;
#endif

    TickType_t last_frame_time_used = 0;
    uint8_t wait_notify_result      = pdFALSE;
    uint8_t num_free_frames         = 0;

    set_gpio_reserve_1_async( 0 );

    /* wait until all tasks have a handle */
    while ( status->task_handles->http_task_handle == NULL || status->task_handles->FPGA_QSPI_task_handle == NULL ||
            status->task_handles->JPEG_task_handle == NULL || status->task_handles->WIFI_task_handle == NULL )
    {
        void set_led_cyan( led_state_t * led );
        vTaskDelay( 10 );
    }
#ifndef DEVELOPMENT_SET_QSPI_STATIC
    uint64_t time_us_start = 0;
    uint64_t time_delta_us = 0;

    /* init wifi */
    xTaskNotifyIndexed( status->task_handles->WIFI_task_handle, TASK_NOTIFY_WIFI_START_BIT, 0, eSetBits );
    xTaskNotifyWaitIndexed( TASK_NOTIFY_CTRL_WIFI_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifyValueWIFI, portMAX_DELAY );

    /* start qspi timer */
    ESP_ERROR_CHECK( gptimer_start( gptimer ) );

    time_us_start = esp_timer_get_time();

#endif

    if ( CTRL_TASK_VERBOSE ) ESP_LOGI( STAT_CTRL_TAG, "Enter Loop" );
    while ( 1 )
    {
#ifndef DEVELOPMENT_SET_QSPI_STATIC

        if ( CTRL_TASK_VERBOSE ) ESP_LOGI( STAT_CTRL_TAG, "last frame: %" PRIu32, last_frame_time_used );
        while ( !wifi_is_connected() )
        {
            if ( CTRL_TASK_VERBOSE ) ESP_LOGI( STAT_CTRL_TAG, "WIFI down, waiting" );
            vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        }

        set_gpio_reserve_1_async( 1 );

        num_free_frames = fifo_has_free_frame();
        if ( num_free_frames != 0 )
        {
            /* Start Http and JPEG */
            time_delta_us = esp_timer_get_time() - time_us_start;
            /* calculate last frame time used, rounding */
            if ( ulNotifyValueHTTP > 0 ) last_frame_time_used = ( uint32_t ) ( ( ( uint32_t ) time_delta_us + 500 ) / 1000 );
            else last_frame_time_used = 0;

            time_us_start = esp_timer_get_time();
            // ESP_LOGI( STAT_CTRL_TAG, "freeframes: %" PRIu8, num_free_frames );
            xTaskNotifyIndexed( status->task_handles->http_task_handle, TASK_NOTIFY_HTTP_START_BIT, last_frame_time_used, eSetBits );
        }
#endif

        if ( STAT_CTRL_ENABLE_LED )
        {
            status->led.toggle_state = !status->led.toggle_state;

            if ( ulNotifyValueHTTP > 0 )
            {
                setup_led_color( &status->led, 0, 128, 0 );
                if ( status->led.toggle_state ) update_led( &status->led );
                else clear_led( &status->led );
            }
            else
            {

#ifdef DEVELOPMENT_SET_QSPI_STATIC
                if ( status->led.toggle_state ) set_led_red( &status->led );
                else clear_led( &status->led );
                gpio_set_level( STAT_CTRL_PIN_RESERVE_3, status->led.toggle_state );
                qspi_request_frame( 0 );

#else
                set_led_red( &status->led );
#endif
            }
        }

#ifndef DEVELOPMENT_SET_QSPI_STATIC

        if ( num_free_frames != 0 )
        {
            /* Wait for QSPI
             * No Clear on Entry, clear on Exit
             */
            // wait_notify_result = xTaskNotifyWaitIndexed( TASK_NOTIFY_CTRL_HTTP_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifyValueQSPI, portMAX_DELAY
            // );
            wait_notify_result =
                xTaskNotifyWaitIndexed( TASK_NOTIFY_CTRL_HTTP_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifyValueHTTP, pdMS_TO_TICKS( 120 ) );
            if ( wait_notify_result == pdFALSE )
            {
                ESP_LOGW( STAT_CTRL_TAG, "Fell through HTTP Finished Waiting" );
            }

            /* Wait for JPEG
             * No Clear on Entry, clear on Exit
             */
            wait_notify_result =
                xTaskNotifyWaitIndexed( TASK_NOTIFY_CTRL_JPEG_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifyValueJPEG, pdMS_TO_TICKS( 70 ) );

            if ( wait_notify_result == pdFALSE )
            {
                ESP_LOGW( STAT_CTRL_TAG, "Fell through JPEG Finished Waiting" );
            }
        }
        else vTaskDelay( pdMS_TO_TICKS( 10 ) );

        set_gpio_reserve_1_async( 0 );
#endif

#ifdef DEVELOPMENT_SET_QSPI_STATIC
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( xPeriod_ms ) );
#endif
    }
    vTaskDelete( NULL );
}
