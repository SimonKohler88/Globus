/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    i2c.h
 * @brief   This file contains all the function prototypes for
 *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN Private defines */
#define I2C_NUM_ADDRESS    ( 8 )
#define I2C_ADDR_LED_BLINK ( 0 )
#define I2C_ADDR_MOT_DUTY_CYCLE_SET ( 1 )
#define I2C_ADDR_MOT_DUTY_CYCLE_IS ( 2 )
#define I2C_ADDR_MOT_DUTY_CYCLE_SLOPE_PER_S ( 3 )

/* USER CODE END Private defines */

void MX_I2C1_Init(void);

/* USER CODE BEGIN Prototypes */
    enum
    {
        I2C_ENTRY_TYPE_READ,
        I2C_ENTRY_TYPE_WRITE,
        I2C_ENTRY_TYPE_READ_WRITE,
    }; typedef uint8_t ENTRY_TYPE_e;

    typedef struct
    {
        uint32_t *val_ptr;
        ENTRY_TYPE_e type;
    } I2C_DATA_ENTRY_t;

    typedef struct
    {
        volatile I2C_DATA_ENTRY_t data[ I2C_NUM_ADDRESS ];
        uint8_t num_entries;
        volatile uint8_t has_update;
    } I2C_DATA_MEM_t;

    void i2c_init( I2C_DATA_MEM_t* data_mem );
    uint8_t i2c_update();
    void i2c_enable_irq( void );
    void i2c_disable_irq( void );

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

