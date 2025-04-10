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
#include "qspi.h"
#include "rotor_encoding.h"
#include "status_control_task_helper.h"

#include "PSRAM_FIFO.h"
#include "wifi.h"

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

void status_control_init( status_control_status_t* status_ptr, command_control_task_t* internal_status_ptr, fifo_status_t* fifo_status )
{
    ESP_LOGI( STAT_CTRL_TAG, "Initializing status control..." );
    internal_status_ptr->status = status_ptr;
    status                      = internal_status_ptr;
    status->fifo_status         = fifo_status;

    gpio_config_t config = {
        .intr_type = GPIO_INTR_POSEDGE, .mode = GPIO_MODE_INPUT, .pull_up_en = 1, .pin_bit_mask = 1 << STAT_CTRL_PIN_FRAME_REQUEST };

    ESP_ERROR_CHECK( gpio_config( &config ) );
    ESP_ERROR_CHECK( gpio_install_isr_service( ESP_INTR_FLAG_IRAM ) );
    ESP_ERROR_CHECK( gpio_isr_handler_add( STAT_CTRL_PIN_FRAME_REQUEST, frame_request_isr_cb, ( void* ) STAT_CTRL_PIN_FRAME_REQUEST ) );
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

void set_gpio_reserve_1_async(uint8_t value)
{
    gpio_set_level( STAT_CTRL_PIN_RESERVE_1, !!value );
}

void status_control_task( void* pvParameter )
{
    ESP_LOGI( STAT_CTRL_TAG, "status_control_task start" );
    if ( status == NULL )
    {
        ESP_LOGE( STAT_CTRL_TAG, "No status struct initialized" );
        return;
    }

    TickType_t xLastWakeTime    = xTaskGetTickCount();
    const TickType_t xPeriod_ms = 50;

    /* for toggling led */
    TickType_t time    = pdTICKS_TO_MS( xTaskGetTickCount() );
    uint32_t led_color = 0x01000041;

    status_control_command_t cmd_buf;

    while ( 1 )
    {
        // if not enough frames -> trigger one
        fifo_update_stats();
        // uint8_t num_free_prog = status->fifo_status->current_frame_2_esp;
        // // if not enough frames -> trigger one
        // if( wifi_is_connected() )
        // {
        //     /* fifo gets us only one. this will be released by wifitask */
        //     if( fifo_has_free_frame() )
        //     {
        //         ESP_LOGI( STAT_CTRL_TAG, "got frame" );
        //         wifi_request_frame( );
        //     }
        // }
        uint8_t num_free_prog = status->fifo_status->current_frame_2_esp;
        uint8_t num_free      = status->fifo_status->free_frames;
        uint8_t num_fpga      = status->fifo_status->ready_4_fpga_frames;
        uint8_t num_fpga_prog = status->fifo_status->current_frame_2_fpga;
        uint8_t num_frames    = num_free + num_fpga + num_free_prog + num_fpga_prog;
        if ( num_frames != 3 )
        {
            ESP_LOGW( STAT_CTRL_TAG, "nfree: %d, nfpga: %d, pfree: %d, pfpga: %d", num_free, num_fpga, num_free_prog, num_fpga_prog );
        }
        // else
        // {
        //     ESP_LOGI( STAT_CTRL_TAG, "nfree: %d, nfpga: %d, pfree: %d, pfpga: %d", num_free, num_fpga, num_free_prog, num_fpga_prog);
        // }
        // handle commands
        uint8_t cmd_waiting = uxQueueMessagesWaiting( status->command_queue_handle );
        for ( uint8_t i = 0; i < cmd_waiting; i++ )
        {
            xQueueReceive( status->command_queue_handle, &cmd_buf, 0 );
            // TODO: handle commands
        }

        if ( pdTICKS_TO_MS( xTaskGetTickCount() ) - time > 100 )
        {
            time         = pdTICKS_TO_MS( xTaskGetTickCount() );
            uint8_t temp = ( led_color >> 31 );
            led_color    = led_color << 1;
            led_color |= temp;
            set_led( status, ( led_color >> 16 ), ( led_color >> 8 ), led_color );
        }

        vTaskDelayUntil( &xLastWakeTime, xPeriod_ms );
    }
    vTaskDelete( NULL );
}
