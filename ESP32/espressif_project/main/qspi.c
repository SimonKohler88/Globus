/*
 * qspi.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "qspi.h"
#include "PSRAM_FIFO.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "hw_settings.h"
#include "portmacro.h"
#include <limits.h>
#include <string.h>

void qspi_post_transaction_cb( spi_transaction_t *trans );

#define QSPI_TAG                            "QSPI"
#define TASK_NOTIFY_QSPI_START_FRAME_BIT    0x01
#define TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT 0x02
// #define TASK_NOTIFY_QSPI_BLOCK_FINISHED_BIT 0x04

// TODO: Check if image can be completely sent or if it works only with Bulks
#define QSPI_MAX_TRANSFER_SIZE              32768   // 2**18 / 8
#define QSPI_MAX_TRANSFER_SIZE_BITS         262143  // 2**18 -1

static spi_device_handle_t qspi_handle;
fifo_frame_t *qspi_frame_info = NULL;

static spi_transaction_t FPGA_transaction = {
    .length    = 0,
    .rxlength  = 0,
    .tx_buffer = NULL,
    .rx_buffer = NULL,
    .flags     = SPI_TRANS_MODE_QIO | SPI_TRANS_DMA_BUFFER_ALIGN_MANUAL,
};

static qspi_status_t *status;

TaskHandle_t internal_qspi_task_handle = NULL;

static uint8_t dma_buffer[ QSPI_MAX_TRANSFER_SIZE ];

void qspi_init( qspi_status_t *status_ptr )
{
    status = status_ptr;

    esp_err_t ret;
    ESP_LOGI( QSPI_TAG, "Initializing bus QSPI2..." );

    /* Need to set CS by hand, because DMA transfer only supports 4kB writes, we
     * need ~100kB */
    // ESP_ERROR_CHECK( gpio_set_direction( QSPI_PIN_CS0, GPIO_MODE_OUTPUT ) );
    // gpio_set_level( QSPI_PIN_CS0, 1 );

    spi_device_interface_config_t FPGA_device_interface_config = {
        .command_bits   = 0,
        .address_bits   = 0,
        .dummy_bits     = 0,
        .mode           = 0,
        .duty_cycle_pos = 0,
        .clock_speed_hz = QSPI_BUS_FREQ,
        .input_delay_ns = 0,
        .spics_io_num   = -1,
        //.cs_ena_pretrans = 5,
        .queue_size     = 1,
        .post_cb        = qspi_post_transaction_cb,
        .flags          = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_HALFDUPLEX,
    };

    spi_bus_config_t buscfg = {
        .data0_io_num    = QSPI_PIN_D_D0,
        .data1_io_num    = QSPI_PIN_Q_D1,
        .data2_io_num    = QSPI_PIN_WP_D2,
        .data3_io_num    = QSPI_PIN_HD_D3,
        .sclk_io_num     = QSPI_PIN_CLK,
        .data4_io_num    = -1,  ///< GPIO pin for spi data4 signal in octal mode,
                                ///< or -1 if not used.
        .data5_io_num    = -1,  ///< GPIO pin for spi data5 signal in octal mode,
                                ///< or -1 if not used.
        .data6_io_num    = -1,  ///< GPIO pin for spi data6 signal in octal mode,
                                ///< or -1 if not used.
        .data7_io_num    = -1,
        .flags           = SPICOMMON_BUSFLAG_QUAD | SPICOMMON_BUSFLAG_IOMUX_PINS | SPICOMMON_BUSFLAG_MASTER,
        .max_transfer_sz = IMAGE_TOTAL_BYTE_SIZE,

    };
    // Initialize the SPI bus
    ret = spi_bus_initialize( QSPI_HOST, &buscfg, QSPI_DMA_CHANNEL );
    ESP_ERROR_CHECK( ret );
    ret = spi_bus_add_device( QSPI_HOST, &FPGA_device_interface_config, &qspi_handle );
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK( gpio_set_direction( QSPI_PIN_CS0, GPIO_MODE_OUTPUT ) );
    gpio_set_level( QSPI_PIN_CS0, 0 );
}

BaseType_t qspi_request_frame( void )
{
    /* called from isr */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( internal_qspi_task_handle != NULL )
    {
        xTaskNotifyIndexedFromISR( internal_qspi_task_handle, TASK_NOTIFY_QSPI_START_FRAME_BIT, 0, eSetBits, &xHigherPriorityTaskWoken );
    }
    return xHigherPriorityTaskWoken;
}

void qspi_DMA_write_debug_test( uint8_t *buffer, uint8_t size )
{
    /* Gets a frame pointer from FIFO and initializes QSPI Transfer.
    Callback from DMA informs FIFO of transfer done */
    esp_err_t spi_ret = ESP_OK;

    FPGA_transaction.tx_buffer = buffer;
    FPGA_transaction.length    = size * 8;
    // TODO: ticks to wait?
    // ESP_LOGI( "QSPI", "sending QSPI" );
    spi_ret                    = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );

    if ( spi_ret != ESP_OK )
    {
        /* Count misses, but no action required */
        if ( spi_ret != ESP_OK )
        {
            status->missed_spi_transfers++;
            ESP_LOGI( "QSPI", "QSPI transfer missed" );
        }
    }
}

static esp_err_t qspi_DMA_write( uint8_t *buffer, uint32_t size )
{
    /* Initiates a QSPI transfer of Max Frame size */

    esp_err_t spi_ret = ESP_OK;

    /* increase ptr of size of max transfer*/
    FPGA_transaction.tx_buffer = ( const void * ) dma_buffer;
    FPGA_transaction.length    = size * 8;
    qspi_frame_info->size -= QSPI_MAX_TRANSFER_SIZE;

    spi_ret = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );

    if ( spi_ret != ESP_OK )
    {
        /* Count misses, but no action required */
        status->missed_spi_transfers++;
    }
    return spi_ret;
}

void qspi_post_transaction_cb( spi_transaction_t *trans )
{
    /* From ISR: QSPI Transaction done */

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if ( internal_qspi_task_handle != NULL )
    {
        xTaskNotifyIndexedFromISR( internal_qspi_task_handle, TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT, 0, eSetBits, &xHigherPriorityTaskWoken );
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static esp_err_t copy_and_send_bulk()
{
    esp_err_t spi_ret = ESP_OK;
    if ( qspi_frame_info->size > QSPI_MAX_TRANSFER_SIZE )
    {
        memcpy( dma_buffer, ( const void * ) qspi_frame_info->current_ptr, QSPI_MAX_TRANSFER_SIZE );
        spi_ret = qspi_DMA_write( dma_buffer, QSPI_MAX_TRANSFER_SIZE );
        qspi_frame_info->current_ptr += QSPI_MAX_TRANSFER_SIZE;
        qspi_frame_info->size -= QSPI_MAX_TRANSFER_SIZE;
    }
    else if ( qspi_frame_info->size > 0 )
    {
        memcpy( dma_buffer, ( const void * ) qspi_frame_info->current_ptr, qspi_frame_info->size );
        spi_ret = qspi_DMA_write( dma_buffer, qspi_frame_info->size );
        qspi_frame_info->current_ptr += qspi_frame_info->size;
        qspi_frame_info->size -= qspi_frame_info->size;
    }
    return spi_ret;
}

void fpga_qspi_task( void *pvParameter )
{

    uint32_t ulNotifiedValue;
    BaseType_t xResult;
    internal_qspi_task_handle = xTaskGetCurrentTaskHandle();
    if ( internal_qspi_task_handle == NULL ) ESP_LOGE( "QSPI", "No Task Handle" );

    while ( 1 )
    {
        // wait for ISR to notify us
        xTaskNotifyWaitIndexed( TASK_NOTIFY_QSPI_START_FRAME_BIT, ULONG_MAX, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY );
        uint8_t frame_sent = 0;
        esp_err_t spi_ret  = ESP_OK;

        /* FPGA Requests a frame */
        if ( !fifo_is_frame_2_fpga_in_progress() )
        {
            if ( fifo_has_frame_4_fpga() )
            {
                ESP_LOGI( "QSPI", "Send Frame" );
                qspi_frame_info = fifo_get_frame_4_fpga();
                if ( qspi_frame_info != NULL )
                {
                    gpio_set_level( QSPI_PIN_CS0, 0 );
                    spi_ret = copy_and_send_bulk();
                    if ( spi_ret == ESP_OK ) frame_sent = 1;
                    else ESP_LOGE( "QSPI", "Err Send: %d \nptr: %" PRIx32 "\nsize: %" PRIu32, spi_ret, ( uint32_t ) qspi_frame_info->current_ptr, ( uint32_t ) qspi_frame_info->size );
                }  // 3C0D 7104     0x3FC88000  0x3FD00000
            }
            else if ( qspi_frame_info != NULL )
            {
                /* resend last frame */
                ESP_LOGI( "QSPI", "Resend Last Frame" );
                qspi_frame_info->current_ptr = qspi_frame_info->frame_start_ptr;
                qspi_frame_info->size        = qspi_frame_info->total_size;

                spi_ret = copy_and_send_bulk();
                if ( spi_ret == ESP_OK ) frame_sent = 1;
                else ESP_LOGE( "QSPI", "Err Resend: %d \nptr: %" PRIx32 "\nsize: %" PRIu32, spi_ret, ( uint32_t ) qspi_frame_info->current_ptr, ( uint32_t ) qspi_frame_info->size );
            }
            // cant do anything...
            else ESP_LOGI( "QSPI", "NoFrame, NoSend" );
        }
        else ESP_LOGI( "QSPI", "Already in Progress" );

        if ( frame_sent )
        {
            uint8_t success = 0;
            for ( uint8_t i = 0; i < ( uint8_t ) ( qspi_frame_info->total_size / QSPI_MAX_TRANSFER_SIZE + 1 ); i++ )
            {
                ESP_LOGI( "QSPI", "waiting for finishing transfer" );
                // wait for ISR to notify us
                xResult = xTaskNotifyWaitIndexed( TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifiedValue, 5 );
                if ( xResult == pdTRUE )
                {
                    if ( qspi_frame_info->size == 0 )
                    {
                        success = 1;
                        break;
                    }

                    spi_ret = copy_and_send_bulk();
                    if ( spi_ret != ESP_OK )
                    {
                        success = 0;
                        break;
                    }
                }
                else success = 0; // timed out
            }

            gpio_set_level( QSPI_PIN_CS0, 1 );
            if ( success )
            {
                ESP_LOGI( "QSPI", "success" );
                qspi_frame_info = NULL;
                fifo_mark_frame_4_fpga_done();
            }
            else
            {
                ESP_LOGW( "QSPI", "no success" );
            }
        }
        else
        {
            status->missed_spi_transfers++;
        }
    }
}
