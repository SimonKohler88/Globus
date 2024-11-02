/*
 * qspi.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */


#include "qspi.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "hw_settings.h"
#include "PSRAM_FIFO.h"


void qspi_post_transaction_cb( spi_transaction_t *trans );


#define QSPI_TAG "QSPI"

static spi_device_handle_t qspi_handle;
static fifo_frame_t qspi_frame_info;

static spi_transaction_t FPGA_transaction =
{
	.length = 0,
	.rxlength = 0,
	.tx_buffer = NULL,
	.rx_buffer = NULL,
	.flags = SPI_TRANS_MODE_QIO | SPI_TRANS_DMA_BUFFER_ALIGN_MANUAL,
};

static qspi_status_t *status;

void qspi_init( qspi_status_t *status_ptr )
{
	status = status_ptr;
	
	esp_err_t ret;
    ESP_LOGI( QSPI_TAG, "Initializing bus QSPI2..." );
    
    spi_device_interface_config_t FPGA_device_interface_config =
	{
		.command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,
		.mode = 0,
		.duty_cycle_pos = 0,
		.clock_speed_hz = QSPI_BUS_FREQ,
		.input_delay_ns = 0,
		.spics_io_num = QSPI_PIN_CS0,
		//.cs_ena_pretrans = 5,
		.queue_size = 1,
		.post_cb = qspi_post_transaction_cb,
		.flags = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_HALFDUPLEX,
	};
    
    spi_bus_config_t buscfg = {
        .data0_io_num = QSPI_PIN_D_D0,
        .data1_io_num = QSPI_PIN_Q_D1,
        .data2_io_num = QSPI_PIN_WP_D2,
        .data3_io_num = QSPI_PIN_HD_D3,
        .sclk_io_num = QSPI_PIN_CLK,
        .data4_io_num = -1,    ///< GPIO pin for spi data4 signal in octal mode, or -1 if not used.
    	.data5_io_num = -1,     ///< GPIO pin for spi data5 signal in octal mode, or -1 if not used.
    	.data6_io_num = -1,     ///< GPIO pin for spi data6 signal in octal mode, or -1 if not used.
    	.data7_io_num = -1,
        .flags = SPICOMMON_BUSFLAG_QUAD | SPICOMMON_BUSFLAG_IOMUX_PINS | SPICOMMON_BUSFLAG_MASTER,
        .max_transfer_sz = IMAGE_TOTAL_BYTE_SIZE,
         
    };
    //Initialize the SPI bus    
    ret = spi_bus_initialize( QSPI_HOST, &buscfg, QSPI_DMA_CHANNEL );
    ESP_ERROR_CHECK( ret );
    ret = spi_bus_add_device( QSPI_HOST, &FPGA_device_interface_config, &qspi_handle );
	ESP_ERROR_CHECK( ret );
	
}

void qspi_DMA_write( void )
{
	/* Gets a frame pointer from FIFO and initializes QSPI Transfer. 
	Callback from DMA informs FIFO of transfer done */
	
	uint8_t ret;
	esp_err_t spi_ret = ESP_OK;
	qspi_frame_info.size = 0;
	
	ret = fifo_get_frame_4_fpga( &qspi_frame_info );
	
	if( ret )
	{
		FPGA_transaction.tx_buffer = qspi_frame_info.frame_start_ptr;
		FPGA_transaction.length = qspi_frame_info.size;
		// TODO: ticks to wait?
		spi_ret = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );
		
	}
	
	if ( ( ret == 0 ) || ( spi_ret != ESP_OK ) )
	{
		/* Count misses, but no action required */
		if( !ret ) status->missed_frames ++;
		else if( spi_ret != ESP_OK ) status->missed_spi_transfers ++;
		
	}
}


void qspi_DMA_write_debug_test( uint8_t* buffer, uint8_t size )
{
	/* Gets a frame pointer from FIFO and initializes QSPI Transfer. 
	Callback from DMA informs FIFO of transfer done */
	esp_err_t spi_ret = ESP_OK;

	FPGA_transaction.tx_buffer = buffer;
	FPGA_transaction.length = size * 8;
	// TODO: ticks to wait?
	//ESP_LOGI( "QSPI", "sending QSPI" );
	spi_ret = spi_device_queue_trans( qspi_handle, &FPGA_transaction, 0 );

	if( spi_ret != ESP_OK )
	{
		/* Count misses, but no action required */
		if( spi_ret != ESP_OK ) 
		{
			status->missed_spi_transfers ++;
			ESP_LOGI( "QSPI", "QSPI transfer missed" );
		}
	}
}

void qspi_post_transaction_cb( spi_transaction_t *trans )
{
	/* From ISR: QSPI Transaction done */
	fifo_mark_frame_4_fpga_done( );
	//ESP_LOGI( "QSPI", "QSPI done" );
	
}