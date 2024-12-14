//
// Created by skohl on 14.12.2024.
//

#ifndef INDUCTION_H
#define INDUCTION_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_tim.h"

    typedef struct
    {
        uint32_t frequency;
        uint32_t duty_cycle;
        uint32_t dead_time;
    } INDUCTION_I2C_IF_t;

    typedef struct
    {
        TIM_HandleTypeDef* htim;
        INDUCTION_I2C_IF_t i2c_if;
    } INDUCTION_t;

    void ind_init( INDUCTION_t* inst, TIM_HandleTypeDef* htim );
    void ind_update( INDUCTION_t* inst, uint32_t tick );
#ifdef __cplusplus
}
#endif
#endif  // INDUCTION_H
