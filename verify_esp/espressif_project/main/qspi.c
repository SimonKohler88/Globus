/*
 * qspi.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "qspi.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "http_task.h"
#include <stdint.h>
#include <string.h>

#include "hw_settings.h"
#include "portmacro.h"
#include <limits.h>

void qspi_post_transaction_cb( spi_transaction_t* trans );
void set_cs_gpio( uint8_t state );

static const char* TAG = "QSPI";

static spi_device_handle_t qspi_handle;

static uint8_t buffer[ QSPI_MAX_TRANSFER_SIZE ];

static spi_device_interface_config_t FPGA_device_interface_config;
static spi_bus_config_t qspi_buscfg;
static spi_transaction_t FPGA_transaction = {
    .length    = 0,
    .rxlength  = 0,
    .tx_buffer = NULL,
    .rx_buffer = buffer,
    .flags     = SPI_TRANS_MODE_QIO ,
};

struct
{
    task_handles_t* task_handles;
    qspi_status_t* status;
} typedef qspi_ctrl_t;
static qspi_ctrl_t qspi_ctrl;

TaskHandle_t internal_qspi_task_handle = NULL;

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
    qspi_buscfg.flags           = SPICOMMON_BUSFLAG_QUAD | SPICOMMON_BUSFLAG_IOMUX_PINS | SPICOMMON_BUSFLAG_SLAVE;
    qspi_buscfg.max_transfer_sz = QSPI_MAX_TRANSFER_SIZE;

    // Initialize the SPI bus
    ret = spi_bus_initialize( QSPI_HOST, &qspi_buscfg, QSPI_DMA_CHANNEL );
    ESP_ERROR_CHECK( ret );
    ret = spi_bus_add_device( QSPI_HOST, &FPGA_device_interface_config, &qspi_handle );
    ESP_ERROR_CHECK( ret );
    ESP_ERROR_CHECK( gpio_set_direction( QSPI_PIN_CS0, GPIO_MODE_OUTPUT ) );
}

BaseType_t qspi_request_frame( void )
{
    /* called from isr */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ( qspi_ctrl.task_handles->http_task_handle != NULL )
    {
        xTaskNotifyIndexedFromISR( internal_qspi_task_handle, TASK_NOTIFY_QSPI_START_BIT, 0, eSetBits, &xHigherPriorityTaskWoken );
    }
    return xHigherPriorityTaskWoken;
}

// static esp_err_t qspi_DMA_write( uint8_t* buffer, uint32_t size )
// {
//     esp_err_t spi_ret = ESP_OK;
//
//     FPGA_transaction.tx_buffer = ( const void* ) dma_buffer;
//     FPGA_transaction.length    = size * 8;
//
//     spi_ret = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );
//
//     if ( spi_ret != ESP_OK )
//     {
//         /* Count misses, but no action required */
//         qspi_ctrl.status->missed_spi_transfers++;
//     }
//     return spi_ret;
// }

void qspi_post_transaction_cb( spi_transaction_t* trans )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    /* From DMA-ISR: QSPI Transaction done */
    if ( trans->rxlength > 0 && trans->rx_buffer != NULL )
    {
        if ( qspi_ctrl.task_handles->http_task_handle != NULL )
        {
            copy_buffer( trans->rx_buffer, trans->rxlength );
            xTaskNotifyIndexedFromISR( qspi_ctrl.task_handles->http_task_handle, TASK_NOTIFY_QSPI_FRAME_FINISHED_BIT, 0, eSetBits, &xHigherPriorityTaskWoken );
        }
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
