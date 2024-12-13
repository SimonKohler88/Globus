/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PIN_ONBOARD_LED_GPIO_Port, PIN_ONBOARD_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PIN_ONBOARD_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PIN_ONBOARD_LED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

void gpio_init_onboard_led( GPIO_BLINK_PIN_t* pin )
{

}
void gpio_update_onboard_led( GPIO_BLINK_PIN_t* pin, uint32_t tick )
{
    uint32_t elapsed_ticks = tick - pin->last_tick;
    uint32_t interval_ticks = pin->interval_ticks;

    if (elapsed_ticks >= interval_ticks )
    {
        pin->last_tick = tick;
        uint8_t state = !HAL_GPIO_ReadPin(PIN_ONBOARD_LED_GPIO_Port, PIN_ONBOARD_LED_Pin);
        HAL_GPIO_WritePin(PIN_ONBOARD_LED_GPIO_Port, PIN_ONBOARD_LED_Pin, state);
    }
}


/* USER CODE END 2 */
