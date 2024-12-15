/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc.h
 * @brief   This file contains all the function prototypes for
 *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc2;

/* USER CODE BEGIN Private defines */
#define ADC_IN_1_CHANNEL ( 1 )
#define ADC_IN_2_CHANNEL ( 2 )
#define ADC_IN_3_CHANNEL ( 3 )
/* USER CODE END Private defines */

void MX_ADC2_Init(void);

/* USER CODE BEGIN Prototypes */

    typedef struct
    {
        ADC_HandleTypeDef* hadc;
        uint32_t value_1;
        uint32_t value_2;
        uint32_t value_3;
    } TRIPLE_ADC_t;

    void adc_init( TRIPLE_ADC_t* triple_adc, ADC_HandleTypeDef* hadc );
    void adc_update( TRIPLE_ADC_t* triple_adc );
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

