/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.h
 * @brief   This file contains all the function prototypes for
 *          the gpio.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
    typedef struct
    {
        uint32_t last_tick;
        uint32_t interval_ticks;
    } GPIO_BLINK_PIN_t;
/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
    void gpio_init_onboard_led( GPIO_BLINK_PIN_t* pin );
    void gpio_update_onboard_led( GPIO_BLINK_PIN_t* pin, uint32_t tick );

    void gpio_set_mot_brake(uint8_t state);
    uint8_t gpio_read_mot_fault();
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

