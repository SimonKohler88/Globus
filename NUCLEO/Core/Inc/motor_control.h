//
// Created by skohl on 11.12.2024.
//

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_tim.h"

    typedef struct
    {
        TIM_HandleTypeDef* htim;

        uint32_t current_speed;
        uint32_t target_speed;
        uint32_t slope;
    } MOT_CTRL_t;

    void mot_ctrl_init( MOT_CTRL_t* mot_ctrl, TIM_HandleTypeDef* htim1 );
    void mot_ctrl_update( MOT_CTRL_t* mot_ctrl );
    void mot_ctrl_set_speed( MOT_CTRL_t* mot_ctrl, float speed );
    uint32_t mot_ctrl_get_speed( MOT_CTRL_t* mot_ctrl );

#ifdef __cplusplus
}
#endif

#endif  // MOTOTR_CONTROL_H
