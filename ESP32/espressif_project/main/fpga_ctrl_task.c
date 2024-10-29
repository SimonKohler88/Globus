/*
 * fpga_ctrl_task.c
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#include "fpga_ctrl_task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "hw_settings.h"
#include "fpga_registers.h"


#define SPI_TAG "SPI"

#define FPGA_STATUS_READ_SIZE_BYTES 1 + sizeof( fpga_status_t )

static void spi_post_transaction_cb( spi_transaction_t *trans );
static void init_spi();

static TaskHandle_t fpga_ctrl_task_handle_internal = NULL;


static spi_device_handle_t spi_handle;
static uint8_t rx_buffer[ FPGA_STATUS_READ_SIZE_BYTES ];
static uint8_t tx_buffer[ FPGA_STATUS_READ_SIZE_BYTES ];
static uint8_t *rx_buffer_ptr = &rx_buffer[ 0 ];
static uint8_t *tx_buffer_ptr = &tx_buffer [ 0 ];


static spi_transaction_t FPGA_sc_transaction =
{
	.flags = 0,
	.length = 0,
	.rxlength = 0,
	.tx_buffer = NULL,
	.rx_buffer = NULL,
};

/* structure and buffer for command queue */
struct {
	uint8_t addr;
	uint32_t value;
} typedef fpga_cmd_t;
static fpga_cmd_t cmd_buffer;


/* static memory allocation for Queues */
StaticQueue_t xQueueBuffer_fpga_cmd;
uint8_t fpga_cmd_queue_buffer[ SPI_CMD_QUEUE_SIZE * sizeof( fpga_cmd_t ) ];

/* internal */
struct {
	fpga_status_t *status;
	QueueHandle_t fpga_cmd_queue_handle;
} typedef fpga_ctrl_t;

static fpga_ctrl_t fpga_ctrl;


void fpga_ctrl_init( fpga_status_t* status )
{
	fpga_ctrl.status = status;
	init_spi();
}

static void init_spi()
{
	esp_err_t ret;
    ESP_LOGI( SPI_TAG, "Initializing bus SPI3..." );
    
    spi_device_interface_config_t FPGA_spi_device_interface_config =
	{
		.command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,
		.mode = 0,
		.duty_cycle_pos = 0,
		.clock_speed_hz = SPI_FREQ,
		.input_delay_ns = 0,
		.spics_io_num = SPI_CS,
		//.cs_ena_pretrans = 5,
		.queue_size = 1,
		.post_cb = spi_post_transaction_cb,
	};
    
    spi_bus_config_t buscfg = {
      	.miso_io_num = SPI_MISO,
      	.mosi_io_num = SPI_MOSI ,
        .sclk_io_num = QSPI_PIN_CLK,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        
        .max_transfer_sz = SPI_MAX_TRANSFER_BYTES,
         
    };
    
    FPGA_sc_transaction.tx_buffer = tx_buffer_ptr;
    FPGA_sc_transaction.rx_buffer = rx_buffer_ptr;
    
    //Initialize the SPI bus
    ret = spi_bus_initialize( SPI_HOST, &buscfg, SPI_DMA_DISABLED );
    ESP_ERROR_CHECK( ret );
    ret = spi_bus_add_device( SPI_HOST, &FPGA_spi_device_interface_config, &spi_handle );
	ESP_ERROR_CHECK( ret );
	
}


static esp_err_t spi_start_read_status( void )
{
	esp_err_t spi_ret;
	tx_buffer[ 0 ] = FPGA_READ_ALL_COMMAND;
	
	FPGA_sc_transaction.length = FPGA_STATUS_READ_SIZE_BYTES;
	
	spi_ret = spi_device_queue_trans( spi_handle, &FPGA_sc_transaction, 2 );
	
	if( spi_ret != ESP_OK ) ESP_LOGI(SPI_TAG, "Read Status Fail: %d", spi_ret);
	return spi_ret;
}

static esp_err_t spi_start_write_command( fpga_cmd_t* command )
{
	esp_err_t spi_ret;
	tx_buffer[ 0 ] = FPGA_WRITE_COMMAND;
	tx_buffer[ 1 ] = command->addr;
	tx_buffer[ 2 ] = command->value; // 4 Byte
	
	FPGA_sc_transaction.length = FPGA_WRITE_COMMAND_SIZE_BYTES;
	
	spi_ret = spi_device_queue_trans( spi_handle, &FPGA_sc_transaction, 2 );
	
	if( spi_ret != ESP_OK ) ESP_LOGI(SPI_TAG, "Read Status Fail: %d", spi_ret);
	return spi_ret;
}

static void spi_readback_buffer_to_struct( void )
{
	// TODO: make this more generic
	fpga_ctrl.status->leds = rx_buffer[ 1 ];
	fpga_ctrl.status->brightness = ( uint32_t ) rx_buffer[ 5 ];
	fpga_ctrl.status->height = ( uint32_t ) rx_buffer[ 9 ];
	fpga_ctrl.status->width = ( uint32_t ) rx_buffer[ 13 ];
}

static void spi_post_transaction_cb( spi_transaction_t *trans )
{
	 BaseType_t xHigherPriorityTaskWoken = pdFALSE;  
	 
	// notify task
	vTaskNotifyGiveFromISR(fpga_ctrl_task_handle_internal, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); 
}


void fpga_ctrl_set_leds( uint8_t leds )
{
	fpga_cmd_t cmd = {
		.addr = FPGA_ADDR_LEDS,
		.value = leds,
	};
	
	xQueueSend( fpga_ctrl.fpga_cmd_queue_handle, (void*)&cmd, 0 );
}

void fpga_ctrl_task( void *pvParameters )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xPeriod_ms = 10;
	esp_err_t err;

	fpga_ctrl.fpga_cmd_queue_handle = xQueueCreateStatic( SPI_CMD_QUEUE_SIZE ,sizeof( fpga_cmd_t ), &fpga_cmd_queue_buffer[ 0 ], &xQueueBuffer_fpga_cmd );
	fpga_ctrl_task_handle_internal = xTaskGetCurrentTaskHandle();
	
	while( 1 )
	{
		vTaskDelayUntil(&xLastWakeTime, xPeriod_ms);
		
		/* Always Read Status */
		err = spi_start_read_status();
		if( err == ESP_OK )
		{
			xTaskNotifyWait( 0x01, 0, NULL, 2); // wait for ISR to notify us
			spi_readback_buffer_to_struct();
		}

		
		/* Write all waiting commands */
		if( uxQueueMessagesWaiting( fpga_ctrl.fpga_cmd_queue_handle ) > 0 )
		{
			while ( xQueueReceive( fpga_ctrl.fpga_cmd_queue_handle, &cmd_buffer, 0 ) )
			{
				err = spi_start_write_command( &cmd_buffer );
				if( err == ESP_OK )
				{
					xTaskNotifyWait( 0x01, 0, NULL, 1); // wait for ISR to notify us
				}		
			}
		}	
	}
}
