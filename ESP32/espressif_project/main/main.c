#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "esp_log.h"
// #include "led_strip.h"
#include "sdkconfig.h"

#include "misc_task.h"

#include "PSRAM_FIFO.h"
#include "fpga_ctrl_task.h"
#include "qspi.h"
#include "rpi_interface.h"
#include "status_control_task.h"
#include "wifi.h"

void vApplicationIdleHook( void );

/* Static Allocations for Freertos task instead of dynamic */
StaticTask_t xWifiTaskBuffer;
StackType_t xWifiStack[ FREERTOS_STACK_SIZE_WIFI ];
TaskHandle_t wifi_task_handle = NULL;

StaticTask_t xWifiSendTaskBuffer;
StackType_t xWifiSendStack[ FREERTOS_STACK_SIZE_WIFI ];
TaskHandle_t wifi_send_task_handle = NULL;

StaticTask_t xFPGACtrlTaskBuffer;
StackType_t xFPGACtrlStack[ FREERTOS_STACK_SIZE_FPGA_CTRL ];
TaskHandle_t FPGA_ctrl_task_handle = NULL;

StaticTask_t xStatusControlTaskBuffer;
StackType_t xStatusControlStack[ FREERTOS_STACK_SIZE_STATUS_CTRL ];
TaskHandle_t status_control_task_handle = NULL;

StaticTask_t xFPGAQSPITaskBuffer;
StackType_t xFPGAQSPIStack[ FREERTOS_STACK_SIZE_QSPI ];
TaskHandle_t FPGA_QSPI_task_handle = NULL;

/* Interface structures initialisation */
fifo_status_t fifo_status;
qspi_status_t qspi_status;
fpga_status_t fpga_status;
fpga_task_status_t fpga_task_status;
status_control_status_t status_control_status;

/* Internal structures */
command_control_task_t command_ctrl_task;

void init_system()
{

    ESP_ERROR_CHECK( nvs_flash_init() );

    fifo_init( &fifo_status );
    register_status_struct( ( void * ) &fifo_status, sizeof( fifo_status ) );

    qspi_init( &qspi_status );
    register_status_struct( ( void * ) &qspi_status, sizeof( qspi_status ) );

    fpga_ctrl_init( &fpga_status, &fpga_task_status );
    register_status_struct( ( void * ) &fpga_status, sizeof( fpga_status ) );

    status_control_init( &status_control_status, &command_ctrl_task, &fifo_status );
    // register_status_struct( ( void* ) &status_control_status, sizeof( status_control_status ) );

    // gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK );
    wifi_receive_init();
}

void app_main( void )
{
    init_system();

    FPGA_ctrl_task_handle = xTaskCreateStaticPinnedToCore( fpga_ctrl_task,                /* Function that implements the task. */
                                                           "fpga_ctrl_task",              /* Text name for the task. */
                                                           FREERTOS_STACK_SIZE_FPGA_CTRL, /* Number of indexes in the xStack array. */
                                                           ( void * ) 1,                  /* Parameter passed into the task. */
                                                           tskIDLE_PRIORITY + 6,          /* Priority at which the task is created. */
                                                           xFPGACtrlStack,                /* Array to use as the task's stack. */
                                                           &xFPGACtrlTaskBuffer,          /* Variable to hold the task's data structure. */
                                                           0 );                           /* Core which executes the task*/

    FPGA_QSPI_task_handle = xTaskCreateStaticPinnedToCore( fpga_qspi_task,                  /* Function that implements the task. */
                                                           "fpga_qspi_task",                /* Text name for the task. */
                                                           FREERTOS_STACK_SIZE_QSPI, /* Number of indexes in the xStack array. */
                                                           ( void * ) 1,                    /* Parameter passed into the task. */
                                                           tskIDLE_PRIORITY + 4,            /* Priority at which the task is created. */
                                                           xFPGAQSPIStack,                  /* Array to use as the task's stack. */
                                                           &xFPGAQSPITaskBuffer,            /* Variable to hold the task's data structure. */
                                                           0 );                             /* Core which executes the task*/

    status_control_task_handle = xTaskCreateStaticPinnedToCore( status_control_task,             /* Function that implements the task. */
                                                                "stat_ctrl_task",                /* Text name for the task. */
                                                                FREERTOS_STACK_SIZE_STATUS_CTRL, /* Number of indexes in the xStack array. */
                                                                ( void * ) 1,                    /* Parameter passed into the task. */
                                                                tskIDLE_PRIORITY + 6,            /* Priority at which the task is created. */
                                                                xStatusControlStack,             /* Array to use as the task's stack. */
                                                                &xStatusControlTaskBuffer,       /* Variable to hold the task's data structure. */
                                                                0 );                             /* Core which executes the task*/

    wifi_task_handle = xTaskCreateStaticPinnedToCore( wifi_receive_udp_task,    /* Function that implements the task. */
                                                      "udp_rcv_task",           /* Text name for the task. */
                                                      FREERTOS_STACK_SIZE_WIFI, /* Number of indexes in the xStack array. */
                                                      ( void * ) 1,             /* Parameter passed into the task. */
                                                      tskIDLE_PRIORITY + 6,     /* Priority at which the task is created. */
                                                      xWifiStack,               /* Array to use as the task's stack. */
                                                      &xWifiTaskBuffer,         /* Variable to hold the task's data structure. */
                                                      1 );                      /* Core which executes the task*/

    wifi_send_task_handle = xTaskCreateStaticPinnedToCore( wifi_send_udp_task,       /* Function that implements the task. */
                                                           "udp_tx_task",            /* Text name for the task. */
                                                           FREERTOS_STACK_SIZE_WIFI, /* Number of indexes in the xStack array. */
                                                           ( void * ) 1,             /* Parameter passed into the task. */
                                                           tskIDLE_PRIORITY + 4,     /* Priority at which the task is created. */
                                                           xWifiSendStack,           /* Array to use as the task's stack. */
                                                           &xWifiSendTaskBuffer,     /* Variable to hold the task's data structure. */
                                                           1 );                      /* Core which executes the task*/
}

void vApplicationIdleHook( void ) {}
