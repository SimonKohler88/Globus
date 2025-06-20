#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "http_task.h"
#include "jpeg2raw.h"
#include "pic_buffer.h"
#include "qspi.h"
#include "status_control_task.h"
#include "wifi.h"
#include "psram_fifo.h"

void vApplicationIdleHook( void );

/* Static Allocations for Freertos task instead of dynamic */
StaticTask_t xHttpTaskBuffer;
StackType_t xHttpTaskStack[ FREERTOS_STACK_SIZE_HTTP ];

StaticTask_t xStatusControlTaskBuffer;
StackType_t xStatusControlStack[ FREERTOS_STACK_SIZE_STATUS_CTRL ];

StaticTask_t xFPGAQSPITaskBuffer;
StackType_t xFPGAQSPIStack[ FREERTOS_STACK_SIZE_QSPI ];

StaticTask_t xJPEGTaskBuffer;
StackType_t xJPEGStack[ FREERTOS_STACK_SIZE_JPEG ];

StaticTask_t xWIFITaskBuffer;
StackType_t xWIFIStack[ FREERTOS_STACK_SIZE_WIFI ];


/* Structures initialisation */
static task_handles_t task_handles;  // for inter-task communication
static buff_status_t buff_status;
static fifo_status_t fifo_status;
static buffer_control_t buff_ctrl;
static qspi_status_t qspi_status;
static status_control_status_t status_control_status;
static command_control_task_t command_ctrl_task;

/*
 * Init all tasks and subtasks
 *
 * @brief
 *
 * @return void
 */
static void init_system( void );
void init_system()
{

    ESP_ERROR_CHECK( nvs_flash_init() );

    buff_ctrl_init( &buff_ctrl, &buff_status );

    fifo_init( &fifo_status );
    // register_status_struct( ( void* ) &fifo_status, sizeof( fifo_status ) );

    qspi_init( &qspi_status, &task_handles );
    // register_status_struct( ( void* ) &qspi_status, sizeof( qspi_status ) );

    // fpga_ctrl_init( &fpga_status, &fpga_task_status );
    // register_status_struct( ( void* ) &fpga_status, sizeof( fpga_status ) );

    status_control_init( &status_control_status, &command_ctrl_task, &task_handles );
    // register_status_struct( ( void* ) &status_control_status, sizeof( status_control_status ) );

    init_http_stat( &task_handles );

    jpeg_init( &task_handles );
    // gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK );
    // wifi_receive_init();

}


void app_main( void )
{
    init_system();

    task_handles.WIFI_task_handle =
        xTaskCreateStaticPinnedToCore( wifi_receive_init_task,             /* Function that implements the task. */
                                       "wifi_task",                /* Text name for the task. */
                                       FREERTOS_STACK_SIZE_WIFI, /* Number of indexes in the xStack array. */
                                       &task_handles,                     /* Parameter passed into the task. */
                                       tskIDLE_PRIORITY + 5,            /* Priority at which the task is created. */
                                       xWIFIStack,             /* Array to use as the task's stack. */
                                       &xWIFITaskBuffer,       /* Variable to hold the task's data structure. */
                                       0 );                             /* Core which executes the task*/


    task_handles.status_control_task_handle =
        xTaskCreateStaticPinnedToCore( status_control_task,             /* Function that implements the task. */
                                       "stat_ctrl_task",                /* Text name for the task. */
                                       FREERTOS_STACK_SIZE_STATUS_CTRL, /* Number of indexes in the xStack array. */
                                       ( void* ) 1,                     /* Parameter passed into the task. */
                                       tskIDLE_PRIORITY + 5,            /* Priority at which the task is created. */
                                       xStatusControlStack,             /* Array to use as the task's stack. */
                                       &xStatusControlTaskBuffer,       /* Variable to hold the task's data structure. */
                                       0 );                             /* Core which executes the task*/



    task_handles.FPGA_QSPI_task_handle = xTaskCreateStaticPinnedToCore( fpga_qspi_task,           /* Function that implements the task. */
                                                                        "fpga_qspi_task",         /* Text name for the task. */
                                                                        FREERTOS_STACK_SIZE_QSPI, /* Number of indexes in the xStack array. */
                                                                        ( void* ) 1,              /* Parameter passed into the task. */
                                                                        tskIDLE_PRIORITY + 4,     /* Priority at which the task is created. */
                                                                        xFPGAQSPIStack,           /* Array to use as the task's stack. */
                                                                        &xFPGAQSPITaskBuffer,     /* Variable to hold the task's data structure. */
                                                                        0 );                      /* Core which executes the task*/



    task_handles.http_task_handle = xTaskCreateStaticPinnedToCore( http_task,                /* Function that implements the task. */
                                                                   "http_task",              /* Text name for the task. */
                                                                   FREERTOS_STACK_SIZE_HTTP, /* Number of indexes in the xStack array. */
                                                                   ( void* ) 1,              /* Parameter passed into the task. */
                                                                   tskIDLE_PRIORITY + 10,    /* Priority at which the task is created. */
                                                                   xHttpTaskStack,           /* Array to use as the task's stack. */
                                                                   &xHttpTaskBuffer,         /* Variable to hold the task's data structure. */
                                                                   0 );                      /* Core which executes the task*/


    task_handles.JPEG_task_handle = xTaskCreateStaticPinnedToCore( jpeg_task,                /* Function that implements the task. */
                                                                   "jpeg_task",              /* Text name for the task. */
                                                                   FREERTOS_STACK_SIZE_JPEG, /* Number of indexes in the xStack array. */
                                                                   ( void* ) 1,              /* Parameter passed into the task. */
                                                                   tskIDLE_PRIORITY + 6,     /* Priority at which the task is created. */
                                                                   xJPEGStack,               /* Array to use as the task's stack. */
                                                                   &xJPEGTaskBuffer,         /* Variable to hold the task's data structure. */
                                                                   1 );
}

void vApplicationIdleHook( void ) {}
