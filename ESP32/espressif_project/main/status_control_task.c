/*
 * status_control_task.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "status_control_task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "fpga_ctrl_task.h"
#include "hw_settings.h"
#include "pic_buffer.h"
#include "qspi.h"
#include "rotor_encoding.h"
#include "status_control_task_helper.h"

// TODO: make all
//  init frame request, reserve etc

static command_control_task_t* status = NULL;

#define STAT_CTRL_TAG "status_control_task"

StaticQueue_t xQueueBuffer_command_queue;
uint8_t command_queue_storage[ STAT_CTRL_QUEUE_NUMBER_OF_COMMANDS * sizeof( status_control_command_t ) ];

volatile uint8_t line_toggle = 0;

static void IRAM_ATTR frame_request_isr_cb( void* arg )
{
    gpio_set_level( STAT_CTRL_PIN_RESERVE_3, line_toggle );
    line_toggle = !line_toggle;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xHigherPriorityTaskWoken            = qspi_request_frame();
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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
    ESP_ERROR_CHECK( gpio_install_isr_service( ESP_INTR_FLAG_IRAM ) );
    // ESP_ERROR_CHECK( gpio_isr_handler_add( STAT_CTRL_PIN_FRAME_REQUEST, frame_request_isr_cb, ( void* ) STAT_CTRL_PIN_FRAME_REQUEST ) );
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

    internal_status_ptr->command_queue_handle = xQueueCreateStatic( STAT_CTRL_QUEUE_NUMBER_OF_COMMANDS, sizeof( status_control_command_t ),
                                                                    &command_queue_storage[ 0 ], &xQueueBuffer_command_queue );

    init_led( internal_status_ptr );
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
    uint8_t is_first_time = 1;
    uint32_t ulNotifyValueJPEG;
    uint32_t ulNotifyValueQSPI;

    TickType_t xLastWakeTime    = xTaskGetTickCount();
    const TickType_t xPeriod_ms = 50;

    /* for toggling led */
    uint32_t led_color              = 0x01000041;
    TickType_t last_frame_time_used = 0;

    status_control_command_t cmd_buf;

    while ( status->task_handles->http_task_handle == NULL || status->task_handles->FPGA_QSPI_task_handle == NULL ||
            status->task_handles->JPEG_task_handle == NULL )
    {
        vTaskDelay( 1 );
    }
    TickType_t time = 0;

    while ( 1 )
    {
        time = pdTICKS_TO_MS( xTaskGetTickCount() );
        /* toggle buffer */
        if ( !is_first_time )
        {
            buff_ctrl_toggle_buff();
        }

        /* Start Http and JPEG */
        // start jpeg
        xTaskNotifyIndexed( status->task_handles->JPEG_task_handle, TASK_NOTIFY_JPEG_START_BIT, 0, eSetBits, );
        // start http
        xTaskNotifyIndexed( status->task_handles->http_task_handle, TASK_NOTIFY_HTTP_START_BIT, last_frame_time_used, eSetBits, );

        // handle commands
        uint8_t cmd_waiting = uxQueueMessagesWaiting( status->command_queue_handle );
        for ( uint8_t i = 0; i < cmd_waiting; i++ )
        {
            xQueueReceive( status->command_queue_handle, &cmd_buf, 0 );
            // TODO: handle commands
        }

        if ( STAT_CTRL_ENABLE_LED )
        {
            // time         = pdTICKS_TO_MS( xTaskGetTickCount() );
            uint8_t temp = ( led_color >> 31 );
            led_color    = led_color << 1;
            led_color |= temp;
            set_led( status, ( led_color >> 16 ), ( led_color >> 8 ), led_color );
        }

        /* Wait for QSPI
         * No Clear on Entry, clear on Exit
         */
        xTaskNotifyWaitIndexed( TASK_NOTIFY_CTRL_QSPI_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifyValueQSPI, portMAX_DELAY );
        // Todo: react on errors

        /* Wait for JPEG
         * No Clear on Entry, clear on Exit
         */
        xTaskNotifyWaitIndexed( TASK_NOTIFY_CTRL_JPEG_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifyValueJPEG, portMAX_DELAY );
        // Todo: react on errors

        is_first_time = 0;
        last_frame_time_used = pdTICKS_TO_MS( xTaskGetTickCount() ) - time;
    }
    vTaskDelete( NULL );
}
