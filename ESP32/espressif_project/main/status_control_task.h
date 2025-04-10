/*
 * status_control_task.h
 *
 *  Created on: 27.10.2024
 *      Author: skohl
 */

#ifndef MAIN_STATUS_CONTROL_TASK_H_
#define MAIN_STATUS_CONTROL_TASK_H_

#include "PSRAM_FIFO.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "led_strip.h"
#include "stdint.h"

/* Structure for Interface */
struct
{

} typedef status_control_status_t;

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

enum
{
    WIFI_TFTP_IDLE,
    WIFI_TFTP_FRAME_REQUESTED,
    WIFI_TFTP_IN_PROGRESS,
};
typedef uint8_t wifi_tftp_state_t;

/* Internal Structure */
struct
{
    status_control_status_t* status;
    QueueHandle_t command_queue_handle;
    fifo_status_t* fifo_status;
    wifi_tftp_state_t wifi_tftp_state;

    /* LED Blink */
    led_strip_handle_t led_strip;
    uint8_t s_led_state;

} typedef command_control_task_t;

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
void status_control_init( status_control_status_t* status_ptr, command_control_task_t* internal_status_ptr, fifo_status_t* fifo_status );



void set_gpio_reserve_1_async(uint8_t value);


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
