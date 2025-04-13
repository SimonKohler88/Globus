/*
 * qspi.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "qspi.h"
#include "pic_buffer.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "hw_settings.h"
#include "portmacro.h"
#include <limits.h>
#include <string.h>

void qspi_post_transaction_cb( spi_transaction_t* trans );
void set_cs_gpio( uint8_t state );

#define QSPI_TAG                            "QSPI"
#define TASK_NOTIFY_QSPI_START_FRAME_BIT    0x01
#define TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT 0x02

#define QSPI_MAX_TRANSFER_SIZE              32768   // 2**18 / 8
#define QSPI_MAX_TRANSFER_SIZE_BITS         262143  // 2**18 -1

static spi_device_handle_t qspi_handle;
frame_unpacked_t* qspi_frame_info = NULL;

static spi_device_interface_config_t FPGA_device_interface_config;
static spi_bus_config_t qspi_buscfg;
static spi_transaction_t FPGA_transaction = {
    .length    = 0,
    .rxlength  = 0,
    .tx_buffer = NULL,
    .rx_buffer = NULL,
    .flags     = SPI_TRANS_MODE_QIO | SPI_TRANS_DMA_BUFFER_ALIGN_MANUAL,
};

static qspi_status_t* status;

TaskHandle_t internal_qspi_task_handle = NULL;

static uint8_t dma_buffer[ QSPI_MAX_TRANSFER_SIZE ];

// static uint8_t test_data[ 259 ] = {
//     0x21, 0x93, 0xd8, 0x22, 0x93, 0xda, 0x22, 0x93, 0xdc, 0x24, 0x94, 0xdf, 0x25, 0x94, 0xe1, 0x25, 0x93, 0xe2, 0x21, 0x94, 0xe2, 0x1e, 0x93, 0xe0,
//     0x1f, 0x96, 0xe0, 0x21, 0x98, 0xdf, 0x20, 0x98, 0xdd, 0x20, 0x98, 0xdc, 0x20, 0x96, 0xda, 0x21, 0x94, 0xdb, 0x20, 0x92, 0xda, 0x1f, 0x92, 0xd9,
//     0x1f, 0x91, 0xd9, 0x20, 0x93, 0xda, 0x1f, 0x92, 0xd9, 0x1e, 0x94, 0xd9, 0x20, 0x96, 0xd9, 0x1e, 0x93, 0xd7, 0x1d, 0x93, 0xd6, 0x1d, 0x93, 0xd6,
//     0x1b, 0x91, 0xd5, 0x1a, 0x8f, 0xd1, 0x19, 0x8f, 0xcf, 0x1a, 0x8f, 0xcf, 0x19, 0x8d, 0xcd, 0x18, 0x8d, 0xcc, 0x17, 0x8c, 0xcb, 0x17, 0x8c, 0xca,
//     0x17, 0x8d, 0xc8, 0x1c, 0x92, 0xcc, 0x20, 0x96, 0xd0, 0x22, 0x98, 0xd2, 0x22, 0x98, 0xd2, 0x21, 0x97, 0xd1, 0x1e, 0x98, 0xd6, 0x1b, 0x96, 0xd9,
//     0x1b, 0x95, 0xd8, 0x1e, 0x98, 0xd9, 0x21, 0x99, 0xd9, 0x21, 0x98, 0xd7, 0x23, 0x97, 0xd5, 0x23, 0x95, 0xd2, 0x23, 0x94, 0xd1, 0x23, 0x94, 0xd1,
//     0x26, 0x96, 0xd3, 0x27, 0x97, 0xd3, 0x27, 0x97, 0xd4, 0x28, 0x99, 0xd7, 0x29, 0x99, 0xd8, 0x2a, 0x9a, 0xd8, 0x29, 0x99, 0xd6, 0x27, 0x96, 0xd2,
//     0x27, 0x95, 0xd1, 0x26, 0x97, 0xd3, 0x23, 0x99, 0xd4, 0x23, 0x99, 0xd5, 0x25, 0x9a, 0xd7, 0x24, 0x9a, 0xd6, 0x27, 0x9d, 0xd9, 0x28, 0x9e, 0xda,
//     0x27, 0xa0, 0xde, 0x28, 0xa1, 0xdf, 0x27, 0xa1, 0xe0, 0x28, 0xa0, 0xe1, 0x2c, 0xa4, 0xe5, 0x30, 0xa7, 0xe9, 0x32, 0xa9, 0xed, 0x31, 0xa8, 0xee,
//     0x2e, 0xa5, 0xec, 0x2d, 0xa3, 0xea, 0x2e, 0xa4, 0xea, 0x2e, 0xa3, 0xe8, 0x2f, 0xa3, 0xe7, 0x2d, 0xa2, 0xdf, 0x29, 0xa1, 0xe2, 0x25, 0x9f, 0xe3,
//     0x23, 0x9c, 0xdf, 0x23, 0x98, 0xd9, 0x22, 0x93, 0xd1, 0x24, 0x8f, 0xd3, 0x24, 0x8f, 0xd6, 0x20, 0x90, 0xd6, 0x1d };

void qspi_init( qspi_status_t* status_ptr )
{
    status = status_ptr;

    esp_err_t ret;
    ESP_LOGI( QSPI_TAG, "Initializing bus QSPI2..." );

    /* Need to set CS by hand, because DMA transfer only supports 32kB writes, we
     * need ~100kB */
    // ESP_ERROR_CHECK( gpio_set_direction( QSPI_PIN_CS0, GPIO_MODE_OUTPUT ) );
    // set_cs_gpio( 1 );

    FPGA_device_interface_config.command_bits   = 0;
    FPGA_device_interface_config.address_bits   = 0;
    FPGA_device_interface_config.dummy_bits     = 0;
    FPGA_device_interface_config.mode           = 0;
    FPGA_device_interface_config.duty_cycle_pos = 0;
    FPGA_device_interface_config.clock_speed_hz = QSPI_BUS_FREQ;
    FPGA_device_interface_config.input_delay_ns = 0;
    FPGA_device_interface_config.spics_io_num   = -1;
    // FPGA_device_interface_config.spics_io_num    = QSPI_PIN_CS0;
    // FPGA_device_interface_config.cs_ena_pretrans = 5;
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
        xTaskNotifyIndexedFromISR( internal_qspi_task_handle, TASK_NOTIFY_QSPI_START_FRAME_BIT, 0, eSetBits, &xHigherPriorityTaskWoken );
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

    // ESP_LOGI( "QSPI", "sending QSPI" );
    spi_ret = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );

    if ( spi_ret != ESP_OK )
    {
        /* Count misses, but no action required */
        status->missed_spi_transfers++;
        ESP_LOGI( "QSPI", "QSPI transfer missed" );
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
        status->missed_spi_transfers++;
    }
    return spi_ret;
}

void qspi_post_transaction_cb( spi_transaction_t* trans )
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
    esp_err_t spi_ret  = ESP_OK;
    uint32_t size2send = 0;

    if ( qspi_frame_info->size > QSPI_MAX_TRANSFER_SIZE ) size2send = QSPI_MAX_TRANSFER_SIZE;
    else if ( qspi_frame_info->size > 0 ) size2send = qspi_frame_info->size;
    else return spi_ret;
    buff_ctrl_copy_mem_protected( dma_buffer, ( const void* ) qspi_frame_info->current_ptr, size2send );
    spi_ret = qspi_DMA_write( dma_buffer, size2send );
    qspi_frame_info->current_ptr += size2send;
    qspi_frame_info->size -= size2send;

    return spi_ret;
}

void set_cs_gpio( uint8_t state )
{
    esp_err_t ret;
    ret = gpio_set_level( QSPI_PIN_CS0, state );
    if ( ret != ESP_OK ) ESP_LOGE( "QSPI", "Err Set CS: %d", ret );
}

#define TEST 1
void fpga_qspi_task( void* pvParameter )
{

    uint32_t ulNotifiedValue;
    BaseType_t xResult;
    internal_qspi_task_handle = xTaskGetCurrentTaskHandle();
    if ( internal_qspi_task_handle == NULL ) ESP_LOGE( "QSPI", "No Task Handle" );
    // gpio_set_level( QSPI_PIN_CS0, 1 );
    ESP_ERROR_CHECK( gpio_set_direction( QSPI_PIN_CS0, GPIO_MODE_OUTPUT ) );
    set_cs_gpio( 1 );
    while ( 1 )
    {
        // wait for ISR to notify us
        xTaskNotifyWaitIndexed( TASK_NOTIFY_QSPI_START_FRAME_BIT, ULONG_MAX, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY );
        uint8_t frame_sent = 0;
        esp_err_t spi_ret  = ESP_OK;

#if ( TEST == 0 ) /* No test, Running hot. */
        /* FPGA Requests a frame */
        if ( !fifo_is_frame_2_fpga_in_progress() )
        {
            if ( fifo_has_frame_4_fpga() )
            {
                ESP_LOGI( "QSPI", "Send Frame" );

                qspi_frame_info = fifo_get_frame_4_fpga();
                if ( qspi_frame_info != NULL )
                {
                    qspi_frame_info->current_ptr = qspi_frame_info->frame_start_ptr;
                    qspi_frame_info->size        = qspi_frame_info->total_size;
                    set_cs_gpio( 0 );
                    spi_ret = copy_and_send_bulk();
                    if ( spi_ret == ESP_OK ) frame_sent = 1;
                    else
                        ESP_LOGE( "QSPI", "Err Send: %d \nptr: %" PRIx32 "\nsize: %" PRIu32, spi_ret, ( uint32_t ) qspi_frame_info->current_ptr,
                                  ( uint32_t ) qspi_frame_info->size );
                }
            }
            // else if ( qspi_frame_info != NULL )
            // {
            //     /* resend last frame */
            //     //ESP_LOGI( "QSPI", "Resend Last Frame" );
            //     qspi_frame_info->current_ptr = qspi_frame_info->frame_start_ptr;
            //     qspi_frame_info->size        = qspi_frame_info->total_size;
            //
            //     set_cs_gpio( 0 );
            //     spi_ret = copy_and_send_bulk();
            //     if ( spi_ret == ESP_OK ) frame_sent = 1;
            //     else
            //         ESP_LOGE( "QSPI", "Err Resend: %d \nptr: %" PRIx32 "\nsize: %" PRIu32, spi_ret, ( uint32_t ) qspi_frame_info->current_ptr,
            //                   ( uint32_t ) qspi_frame_info->size );
            // }
            // cant do anything...
            // TODO: maybe send static Frame?
            else ESP_LOGI( "QSPI", "NoFrame, NoSend" );
        }
        else ESP_LOGI( "QSPI", "Already in Progress" );

#elif ( TEST == 1 ) /* Testing with static picture in PSRAM */
        if ( qspi_frame_info == NULL ) qspi_frame_info = buff_ctrl_get_static_frame();
        else
        {
            qspi_frame_info->size        = qspi_frame_info->total_size;
            qspi_frame_info->current_ptr = qspi_frame_info->frame_start_ptr;

            set_cs_gpio( 0 );
            spi_ret = copy_and_send_bulk( qspi_frame_info );
            ESP_LOGI( "QSPI", "sending Frame started totalsize: %" PRIu32 "  size %" PRIu32, qspi_frame_info->total_size, qspi_frame_info->size );
            if ( spi_ret == ESP_OK ) frame_sent = 1;
            else
                ESP_LOGE( "QSPI", "Err Send Test Pic: %d \nptr: %" PRIx32 "\nsize: %" PRIu32, spi_ret, ( uint32_t ) qspi_frame_info->current_ptr,
                          ( uint32_t ) qspi_frame_info->size );
        }

#elif ( TEST == 2 ) /* Testwise sending small buffer. uncomment the static memory on top of the file */
        set_cs_gpio( 0 );
        qspi_DMA_write_debug_test( test_data, 256 );
        xTaskNotifyWaitIndexed( TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifiedValue, 5 );
        set_cs_gpio( 1 );

#endif

        if ( frame_sent )
        {
            // gpio_dump_io_configuration(stdout, 1<<QSPI_PIN_CS0 );
            uint8_t success = 0;
            for ( uint8_t i = 0; i < ( uint8_t ) ( qspi_frame_info->total_size / QSPI_MAX_TRANSFER_SIZE + 1 ); i++ )
            {
                // set_cs_gpio( 0 );
                //ESP_LOGI( "QSPI", "waiting for finishing transfer" );
                //  wait for Post-DMA-ISR to notify us
                xResult = xTaskNotifyWaitIndexed( TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT, pdFALSE, ULONG_MAX, &ulNotifiedValue, 5 );
                if ( xResult == pdTRUE )
                {
                    if ( qspi_frame_info->size == 0 )
                    {
                        success = 1;

                        break;
                    }

                    spi_ret = copy_and_send_bulk();
                    // uint32_t size = qspi_frame_info->total_size;
                    //ESP_LOGI( "QSPI", "next piece totalsize:%"PRIu32" size %"PRIu32" ", size, qspi_frame_info->size );
                    if ( spi_ret != ESP_OK )
                    {
                        success = 0;
                        break;
                    }
                }
                else success = 0;  // timed out
            }
            set_cs_gpio( 1 );
            // fifo_mark_frame_4_fpga_done();

            if ( success )
            {
                ESP_LOGI( "QSPI", "success" );
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
