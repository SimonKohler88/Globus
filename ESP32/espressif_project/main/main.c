#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "esp_log.h"
//#include "led_strip.h"
#include "sdkconfig.h"

#include "misc_task.h"

#include "PSRAM_FIFO.h"
#include "wifi.h"
#include "rpi_interface.h"
#include "qspi.h"
#include "fpga_ctrl_task.h"


void vApplicationIdleHook( void );

/* Static Allocations for Freertos task instead of dynamic */
StaticTask_t xWifiTaskBuffer;
StackType_t xWifiStack[ FREERTOS_STACK_SIZE ];
TaskHandle_t wifi_task_handle = NULL;

StaticTask_t xMiscTaskBuffer;
StackType_t xMiscStack[ FREERTOS_STACK_SIZE ];
TaskHandle_t misc_task_handle = NULL;

StaticTask_t xFPGACtrlTaskBuffer;
StackType_t xFPGACtrlStack[ FREERTOS_STACK_SIZE ];
TaskHandle_t FPGA_ctrl_task_handle = NULL;

/* Interface structures initialisation */
fifo_status_t fifo_status;
qspi_status_t qspi_status;
fpga_status_t fpga_status;


void init_system()
{

	ESP_ERROR_CHECK(nvs_flash_init());
	fifo_init( &fifo_status );
	register_status_struct( ( void* ) &fifo_status, sizeof( fifo_status ) );
	qspi_init( &qspi_status );
	register_status_struct( ( void* ) &qspi_status, sizeof( qspi_status ) );
	fpga_ctrl_init( &fpga_status );
	register_status_struct( ( void* ) &fpga_status, sizeof( fpga_status ) );
	
	//gpio_dump_io_configuration(stdout, ( 1ULL << 35 ) | ( 1ULL << 36 ) | ( 1ULL << 38 ) );
	gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK );
	wifi_receive_init();
}


void debug_task( void *pvParameters )
{
	
}

void app_main(void)
{
	init_system();
	
	wifi_task_handle = xTaskCreateStaticPinnedToCore(
                      wifi_receive_udp_task,       /* Function that implements the task. */
                      "udp_receive_task",          /* Text name for the task. */
                      FREERTOS_STACK_SIZE,      /* Number of indexes in the xStack array. */
                      ( void * ) 1,    /* Parameter passed into the task. */
                      tskIDLE_PRIORITY + 6 ,/* Priority at which the task is created. */
                      xWifiStack,          /* Array to use as the task's stack. */
                      &xWifiTaskBuffer , /* Variable to hold the task's data structure. */
                      1);  /* Core which executes the task*/  
                      
    misc_task_handle = xTaskCreateStaticPinnedToCore(
                      misc_task,       /* Function that implements the task. */
                      "misc_task",          /* Text name for the task. */
                      FREERTOS_STACK_SIZE,      /* Number of indexes in the xStack array. */
                      ( void * ) 1,    /* Parameter passed into the task. */
                      tskIDLE_PRIORITY + 5 ,/* Priority at which the task is created. */
                      xMiscStack,          /* Array to use as the task's stack. */
                      &xMiscTaskBuffer , /* Variable to hold the task's data structure. */
                      0);  /* Core which executes the task*/            

	 FPGA_ctrl_task_handle = xTaskCreateStaticPinnedToCore(
                      fpga_ctrl_task,       /* Function that implements the task. */
                      "fpga_ctrl_task",          /* Text name for the task. */
                      FREERTOS_STACK_SIZE,      /* Number of indexes in the xStack array. */
                      ( void * ) 1,    /* Parameter passed into the task. */
                      tskIDLE_PRIORITY + 5 ,/* Priority at which the task is created. */
                      xFPGACtrlStack,          /* Array to use as the task's stack. */
                      &xFPGACtrlTaskBuffer , /* Variable to hold the task's data structure. */
                      0);  /* Core which executes the task*/      
}

void vApplicationIdleHook( void )
{
	
}
