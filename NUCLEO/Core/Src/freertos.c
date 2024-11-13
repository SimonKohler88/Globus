/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;

typedef struct {

} MainTask_Msg_t;

typedef struct {

} MotorTask_Msg_t;

typedef struct {

} CoilTask_Msg_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define QUEUE_ENTRIES 10
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* static memory allocation for Queues */
StaticQueue_t xQueueBuffer_main_task_msg_queue;
uint8_t main_task_msg_queue_buffer[QUEUE_ENTRIES * sizeof(MainTask_Msg_t)];
QueueHandle_t main_task_msg_queue_handle = xQueueCreateStatic( QUEUE_ENTRIES,
		sizeof(MainTask_Msg_t), &main_task_msg_queue_buffer[0],
		&xQueueBuffer_main_task_msg_queue);

StaticQueue_t xQueueBuffer_motor_task_msg_queue;
uint8_t motor_task_msg_queue_buffer[QUEUE_ENTRIES * sizeof(MotorTask_Msg_t)];
QueueHandle_t motor_task_msg_queue_handle = xQueueCreateStatic( QUEUE_ENTRIES,
		sizeof(MotorTask_Msg_t), &main_motor_msg_queue_buffer[0],
		&xQueueBuffer_motor_task_msg_queue);

StaticQueue_t xQueueBuffer_coil_task_msg_queue;
uint8_t main_coil_msg_queue_buffer[QUEUE_ENTRIES * sizeof(CoilTask_Msg_t)];
QueueHandle_t coil_task_msg_queue_handle = xQueueCreateStatic( QUEUE_ENTRIES,
		sizeof(CoilTask_Msg_t), &coil_task_msg_queue_buffer[0],
		&xQueueBuffer_coil_task_msg_queue);

/* USER CODE END Variables */
/* Definitions for mainTask */
osThreadId_t mainTaskHandle;
uint32_t mainTaskBuffer[128];
osStaticThreadDef_t mainTaskControlBlock;
const osThreadAttr_t mainTask_attributes =
{ .name = "mainTask", .cb_mem = &mainTaskControlBlock, .cb_size =
		sizeof(mainTaskControlBlock), .stack_mem = &mainTaskBuffer[0],
		.stack_size = sizeof(mainTaskBuffer), .priority =
				(osPriority_t) osPriorityNormal, };
/* Definitions for motorCtrlTask */
osThreadId_t motorCtrlTaskHandle;
uint32_t motorCtrlTaskBuffer[128];
osStaticThreadDef_t motorCtrlTaskControlBlock;
const osThreadAttr_t motorCtrlTask_attributes =
{ .name = "motorCtrlTask", .cb_mem = &motorCtrlTaskControlBlock, .cb_size =
		sizeof(motorCtrlTaskControlBlock), .stack_mem = &motorCtrlTaskBuffer[0],
		.stack_size = sizeof(motorCtrlTaskBuffer), .priority =
				(osPriority_t) osPriorityLow, };
/* Definitions for coilTaskT */
osThreadId_t coilTaskTHandle;
uint32_t coilTaskBuffer[128];
osStaticThreadDef_t coilTaskControlBlock;
const osThreadAttr_t coilTaskT_attributes =
{ .name = "coilTaskT", .cb_mem = &coilTaskControlBlock, .cb_size =
		sizeof(coilTaskControlBlock), .stack_mem = &coilTaskBuffer[0],
		.stack_size = sizeof(coilTaskBuffer), .priority =
				(osPriority_t) osPriorityLow, };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartmaintTask(void *argument);
void motorCtrlTaskEntry(void *argument);
void coilTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of mainTask */
	mainTaskHandle = osThreadNew(StartmaintTask, NULL, &mainTask_attributes);

	/* creation of motorCtrlTask */
	motorCtrlTaskHandle = osThreadNew(motorCtrlTaskEntry, NULL,
			&motorCtrlTask_attributes);

	/* creation of coilTaskT */
	coilTaskTHandle = osThreadNew(coilTask, NULL, &coilTaskT_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartmaintTask */
/**
 * @brief  Function implementing the mainTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartmaintTask */
void StartmaintTask(void *argument)
{
	/* USER CODE BEGIN StartmaintTask */
	MainTask_Msg_t msg_buffer;


	/* Infinite loop */
	for (;;)
	{
		/* Check if Message pending */
		if( uxQueueMessagesWaiting( main_task_msg_queue_handle ) > 0 )
		{
			/* Handle all Msg of other Tasks */
			while ( xQueueReceive( main_task_msg_queue_handle, &msg_buffer, 0 ) )
			{

			}
		}

		/* Wait 4 ms for I2C Interrupt*/
		uint8_t ret = ulTaskNotifyTake( pdTRUE, 4 );
		if( ret == pdTRUE )
		{

		}


	}
	/* USER CODE END StartmaintTask */
}

/* USER CODE BEGIN Header_motorCtrlTaskEntry */
/**
 * @brief Function implementing the motorCtrlTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_motorCtrlTaskEntry */
void motorCtrlTaskEntry(void *argument)
{
	/* USER CODE BEGIN motorCtrlTaskEntry */
	MotorTask_Msg_t msg_buffer;


	/* Infinite loop */
	for (;;)
	{

		/* Handle all Msg of other Tasks */
		uint8_t ret = xQueueReceive( motor_task_msg_queue_handle, &msg_buffer, portMAX_DELAY )
		{

		}
	}
	/* USER CODE END motorCtrlTaskEntry */
}

/* USER CODE BEGIN Header_coilTask */
/**
 * @brief Function implementing the coilTaskT thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_coilTask */
void coilTask(void *argument)
{
	/* USER CODE BEGIN coilTask */
	/* Infinite loop */
	for (;;)
	{
		osDelay(1);
	}
	/* USER CODE END coilTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

