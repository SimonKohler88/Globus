/*
 * qspi.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "qspi.h"

#include <string.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"

#include "hw_settings.h"
#include "portmacro.h"
#include "psram_fifo.h"
#include <limits.h>

void qspi_post_transaction_cb( spi_transaction_t* trans );
void set_cs_gpio( uint8_t state );

static const char* TAG = "QSPI";
#define QSPI_MAX_TRANSFER_SIZE       ( 32768 )   // 2**18 / 8
#define QSPI_MAX_TRANSFER_SIZE_BITS  ( 262143 )  // 2**18 -1

static spi_device_handle_t qspi_handle;
fifo_frame_t* qspi_frame = NULL;

static spi_device_interface_config_t FPGA_device_interface_config;
static spi_bus_config_t qspi_buscfg;
static spi_transaction_t FPGA_transaction = {
    .length    = 0,
    .rxlength  = 0,
    .tx_buffer = NULL,
    .rx_buffer = NULL,
    .flags     = SPI_TRANS_MODE_QIO | SPI_TRANS_DMA_BUFFER_ALIGN_MANUAL,
};

struct
{
    task_handles_t* task_handles;
    qspi_status_t* status;
} typedef qspi_ctrl_t;
static qspi_ctrl_t qspi_ctrl;

TaskHandle_t internal_qspi_task_handle = NULL;

static uint8_t dma_buffer[ QSPI_MAX_TRANSFER_SIZE ];

void qspi_init( qspi_status_t* status_ptr, task_handles_t* task_handles )
{
    qspi_ctrl.status       = status_ptr;
    qspi_ctrl.task_handles = task_handles;

    esp_err_t ret;
    ESP_LOGI( TAG, "Initializing bus QSPI2..." );

    /* Need to set CS by hand, because DMA transfer only supports 32kB writes, we
     * need ~100kB */
    FPGA_device_interface_config.command_bits   = 0;
    FPGA_device_interface_config.address_bits   = 0;
    FPGA_device_interface_config.dummy_bits     = 0;
    FPGA_device_interface_config.mode           = 0;
    FPGA_device_interface_config.duty_cycle_pos = 0;
    FPGA_device_interface_config.clock_speed_hz = QSPI_BUS_FREQ;
    FPGA_device_interface_config.input_delay_ns = 0;
    FPGA_device_interface_config.spics_io_num   = -1;
    FPGA_device_interface_config.queue_size     = 1;
    FPGA_device_interface_config.post_cb        = qspi_post_transaction_cb;
    FPGA_device_interface_config.flags          = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_HALFDUPLEX;

    qspi_buscfg.data0_io_num    = QSPI_PIN_D_D0;
    qspi_buscfg.data1_io_num    = QSPI_PIN_Q_D1;
    qspi_buscfg.data2_io_num    = QSPI_PIN_WP_D2;
    qspi_buscfg.data3_io_num    = QSPI_PIN_HD_D3;
    qspi_buscfg.sclk_io_num     = QSPI_PIN_CLK;
    qspi_buscfg.data4_io_num    = -1;  ///< GPIO pin for spi data4 signal in octal mode,
                                       ///< or -1 if not used.
    qspi_buscfg.data5_io_num    = -1;  ///< GPIO pin for spi data5 signal in octal mode,
                                       ///< or -1 if not used.
    qspi_buscfg.data6_io_num    = -1;  ///< GPIO pin for spi data6 signal in octal mode,
                                       ///< or -1 if not used.
    qspi_buscfg.data7_io_num    = -1;
    qspi_buscfg.flags           = SPICOMMON_BUSFLAG_QUAD | SPICOMMON_BUSFLAG_IOMUX_PINS | SPICOMMON_BUSFLAG_MASTER;
    qspi_buscfg.max_transfer_sz = IMAGE_TOTAL_BYTE_SIZE;

    // Initialize the SPI bus
    ret = spi_bus_initialize( QSPI_HOST, &qspi_buscfg, QSPI_DMA_CHANNEL );
    ESP_ERROR_CHECK( ret );
    ret = spi_bus_add_device( QSPI_HOST, &FPGA_device_interface_config, &qspi_handle );
    ESP_ERROR_CHECK( ret );
    ESP_ERROR_CHECK( gpio_set_direction( QSPI_PIN_CS0, GPIO_MODE_OUTPUT ) );
    set_cs_gpio( 1 );
}

BaseType_t qspi_request_frame( void )
{
    /* called from isr */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( internal_qspi_task_handle != NULL )
    {
        xTaskNotifyIndexedFromISR( internal_qspi_task_handle, TASK_NOTIFY_QSPI_START_BIT, 0, eSetBits, &xHigherPriorityTaskWoken );
    }
    return xHigherPriorityTaskWoken;
}

void qspi_DMA_write_debug_test( uint8_t* buffer, uint32_t size )
{
    /* Gets a frame pointer from FIFO and initializes QSPI Transfer.
    Callback from DMA informs FIFO of transfer done */
    esp_err_t spi_ret = ESP_OK;

    FPGA_transaction.tx_buffer = buffer;
    FPGA_transaction.length    = size * 8;

    // ESP_LOGI( TAG, "sending QSPI" );
    spi_ret = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );

    if ( spi_ret != ESP_OK )
    {
        /* Count misses, but no action required */
        qspi_ctrl.status->missed_spi_transfers++;
        ESP_LOGI( TAG, "QSPI transfer missed" );
    }
}

static esp_err_t qspi_DMA_write( uint8_t* buffer, uint32_t size )
{
    esp_err_t spi_ret = ESP_OK;

    FPGA_transaction.tx_buffer = ( const void* ) dma_buffer;
    FPGA_transaction.length    = size * 8;

    spi_ret = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );

    if ( spi_ret != ESP_OK )
    {
        /* Count misses, but no action required */
        qspi_ctrl.status->missed_spi_transfers++;
    }
    return spi_ret;
}

void qspi_post_transaction_cb( spi_transaction_t* trans )
{
    /* From DMA-ISR: QSPI Transaction done */

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if ( internal_qspi_task_handle != NULL )
    {
        xTaskNotifyIndexedFromISR( internal_qspi_task_handle, TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT, 0, eSetBits, &xHigherPriorityTaskWoken );
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static esp_err_t copy_and_send_bulk()
{
    esp_err_t spi_ret  = ESP_OK;
    uint32_t size2send = 0;

    if ( qspi_frame->size > QSPI_MAX_TRANSFER_SIZE ) size2send = QSPI_MAX_TRANSFER_SIZE;
    else if ( qspi_frame->size > 0 ) size2send = qspi_frame->size;
    else return spi_ret;

    fifo_copy_mem_protected( dma_buffer, ( const void* ) qspi_frame->current_ptr, size2send );
    spi_ret = qspi_DMA_write( dma_buffer, size2send );

    qspi_frame->current_ptr += size2send;
    qspi_frame->size -= size2send;

    return spi_ret;
}

void set_cs_gpio( uint8_t state )
{
    esp_err_t ret;
    ret = gpio_set_level( QSPI_PIN_CS0, state );
    if ( ret != ESP_OK ) ESP_LOGE( TAG, "Err Set CS: %d", ret );
}

#ifndef DEVELOPMENT_SET_QSPI_ON_PIN_OUT
    #define QSPI_TEST 0
#else
    #define QSPI_TEST 1
#endif

void fpga_qspi_task( void* pvParameter )
{
    uint32_t ulNotifiedValue;
    uint32_t ulNotifiedValuefromHTTP;
    BaseType_t xResult;

    internal_qspi_task_handle = xTaskGetCurrentTaskHandle();
    if ( internal_qspi_task_handle == NULL ) ESP_LOGE( TAG, "No Task Handle" );

    ESP_ERROR_CHECK( gpio_set_direction( QSPI_PIN_CS0, GPIO_MODE_OUTPUT ) );
    set_cs_gpio( 1 );

    if ( QSPI_TASK_VERBOSE ) ESP_LOGI( TAG, "Enter Loop" );
    while ( 1 )
    {

        /* wait for Timer (or Pin in DevMode) to notify us
         * clear on Entry, clear on exit
         */
        xTaskNotifyWaitIndexed( TASK_NOTIFY_QSPI_START_BIT, ULONG_MAX, ULONG_MAX, &ulNotifiedValuefromHTTP, portMAX_DELAY );
        if ( QSPI_TASK_VERBOSE ) ESP_LOGI( TAG, "Start" );

        uint8_t frame_sent = 0;
        esp_err_t spi_ret  = ESP_OK;

#if ( QSPI_TEST == 0 ) /* No test, Running hot. */

        qspi_frame = fifo_get_frame_4_fpga();
        if ( qspi_frame != NULL )
        {
            qspi_frame->current_ptr = qspi_frame->frame_start_ptr;
            qspi_frame->size        = qspi_frame->total_size;
            set_cs_gpio( 0 );
            spi_ret = copy_and_send_bulk();
            if ( spi_ret == ESP_OK ) frame_sent = 1;
            else fifo_mark_frame_4_fpga_done();

            if ( QSPI_TASK_VERBOSE ) ESP_LOGI( TAG, "sending Frame started totalsize: %" PRIu32 "  size %" PRIu32, qspi_frame->total_size, qspi_frame->size );
        }
        else
        {
            ESP_LOGW( TAG, "Invalid Frame" );
        }

#elif ( QSPI_TEST == 1 ) /* Testing with static picture in PSRAM */
        if ( qspi_frame == NULL ) qspi_frame = fifo_get_static_frame();
        else
        {
            qspi_frame->size        = qspi_frame->total_size;
            qspi_frame->current_ptr = qspi_frame->frame_start_ptr;

            set_cs_gpio( 0 );
            spi_ret = copy_and_send_bulk( qspi_frame );
            ESP_LOGI( TAG, "sending Frame started totalsize: %" PRIu32 "  size %" PRIu32, qspi_frame->total_size, qspi_frame->size );
            if ( spi_ret == ESP_OK ) frame_sent = 1;
            else
                ESP_LOGE( TAG, "Err Send Test Pic: %d \nptr: %" PRIx32 "\nsize: %" PRIu32, spi_ret, ( uint32_t ) qspi_frame->current_ptr,
                          ( uint32_t ) qspi_frame->size );
        }

#endif

        if ( frame_sent )
        {
            // gpio_dump_io_configuration(stdout, 1<<QSPI_PIN_CS0 );
            uint8_t success = 0;
            for ( uint8_t i = 0; i < ( uint8_t ) ( qspi_frame->total_size / QSPI_MAX_TRANSFER_SIZE + 1 ); i++ )
            {
                if ( QSPI_TASK_VERBOSE ) ESP_LOGI( TAG, "waiting for finishing transfer" );
                //  wait for Post-DMA-ISR to notify us
                xResult = xTaskNotifyWaitIndexed( TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifiedValue, 5 );
                if ( xResult == pdTRUE )
                {
                    if ( qspi_frame->size == 0 )
                    {
                        success = 1;
                        break;
                    }

                    spi_ret = copy_and_send_bulk();
                    if ( spi_ret != ESP_OK )
                    {
                        success = 0;
                        if ( QSPI_TASK_VERBOSE ) ESP_LOGW( TAG, "no success: spi_ret" );
                        break;
                    }
                }
                else
                {
                    success = 0;  // timed out
                    if ( QSPI_TASK_VERBOSE ) ESP_LOGW( TAG, "no success: timeout" );
                }
            }
            set_cs_gpio( 1 );

            if ( success )
            {
                if ( QSPI_TASK_VERBOSE ) ESP_LOGI( TAG, "success" );
            }
            else
            {
                ESP_LOGW( TAG, "no success" );
            }
            fifo_mark_frame_4_fpga_done();
        }
        else
        {
            qspi_ctrl.status->missed_spi_transfers++;
        }

        if ( QSPI_TASK_VERBOSE ) ESP_LOGI( TAG, "Done" );
    }
}
