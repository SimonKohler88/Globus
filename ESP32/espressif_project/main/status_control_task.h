/*
 * status_control_task.h
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#ifndef MAIN_STATUS_CONTROL_TASK_H_
#define MAIN_STATUS_CONTROL_TASK_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "led_strip.h"
#include "pic_buffer.h"
#include "stdint.h"

/* Structure for Interface */
struct
{

} typedef status_control_status_t;

/* Task Handles for intertask communication */
struct
{
    TaskHandle_t http_task_handle;
    TaskHandle_t FPGA_ctrl_task_handle;
    TaskHandle_t status_control_task_handle;
    TaskHandle_t FPGA_QSPI_task_handle;
    TaskHandle_t JPEG_task_handle;

} typedef task_handles_t;

/* Command possibilities for task from ETH */
enum
{
    COMMAND_DEBUG = 0,
    COMMAND_SEND_STATUS,
};
typedef uint8_t COMMANDS_t;

/* Command Structure for Queue */
struct
{
    COMMANDS_t command;
    uint32_t value;
} typedef status_control_command_t;

/* Internal Structure */
struct
{
    status_control_status_t* status;
    QueueHandle_t command_queue_handle;

    task_handles_t* task_handles;

    /* LED Blink */
    led_strip_handle_t led_strip;
    uint8_t s_led_state;

} typedef command_control_task_t;

/* Bit definitions for inter-Task communication Rendevous
 * JPEG-> Ctrl, QSPI->Ctrl */
#define TASK_NOTIFY_CTRL_QSPI_FINISHED_BIT 0x00
#define TASK_NOTIFY_CTRL_JPEG_FINISHED_BIT 0x01

/* Task Notification for Inter Task communication
 * HTTP -> QSPI-Task */
#define TASK_NOTIFY_QSPI_START_BIT          0x00

/* Task Notification for Inter Task communication with HTTP-Task
 * Ctrl -> HTTP
 */
#define TASK_NOTIFY_HTTP_START_BIT          0x00

/* Task Notification for Inter Task communication with JPEG-Task
 * Ctrl -> JPEG
 */
#define TASK_NOTIFY_JPEG_START_BIT          0x00

/**
 * Initializes the status control with the given status pointers.
 *
 * This function sets up the internal status control structures and
 * configures the necessary GPIOs for communication. It specifically
 * configures GPIO for the handling of FPGA control lanes and requests
 * the FPGA to be reset.
 *
 * @param status_ptr Pointer to the status_control_status_t structure
 *                   that holds the control status information.
 * @param internal_status_ptr Pointer to the command_control_task_t
 *                            structure that manages internal command
 *                            and status handling for the task.
 */
void status_control_init( status_control_status_t* status_ptr, command_control_task_t* internal_status_ptr, task_handles_t* task_handles );

void set_gpio_reserve_1_async( uint8_t value );

/**
 * @brief Task function for controlling status operations. This task manages the
 *        LED states and processes command messages from a queue.
 *
 * @param pvParameter A pointer to task parameters, if any, used for task-specific
 *                    needs. Typically used to pass structures or other data
 *                    necessary for task operation. In this function, it might
 *                    relate to status control data.
 */
void status_control_task( void* pvParameter );

#endif /* MAIN_STATUS_CONTROL_TASK_H_ */
