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
#include "tim.h"

// #define MOT_CTRL_TIMER_INPUT_CLOCK_HZ ( 8000000 )
#define MOT_CTRL_TIMER_INPUT_CLOCK_HZ ( 32000000 )
#define MOT_CTRL_PWM_FREQUENCY        ( 4000 )
// #define MOT_CTRL_TIMER_RESOLUTION     ( 1000 )
#define MOT_CTRL_TIMER_RESOLUTION     ( 1500 )
#if MOT_CTRL_TIMER_RESOLUTION > ( 0xFFFF )
    #define MOT_CTRL_TIMER_RESOLUTION ( 0xFFFF )
#endif

    typedef struct
    {
        uint16_t current_speed_duty_cycle;  // pwm
        uint16_t target_speed_duty_cycle;
        uint16_t slope_duty_cycle_per_s;

    } MOT_CTRL_I2C_IF_t;

    typedef struct
    {
        TIM_HandleTypeDef* htim;

        /* I2C Interface */
        MOT_CTRL_I2C_IF_t i2c_if;
        uint32_t slope_duty_cycle_per_hwcycle;
        uint32_t channel;
        uint32_t last_tick;
        float slope_carry_over;
    } MOT_CTRL_t;

    void mot_ctrl_init( MOT_CTRL_t* mot_ctrl, TIM_HandleTypeDef* htim, uint32_t channel );
    void mot_ctrl_update( MOT_CTRL_t* mot_ctrl, uint32_t tick );

#ifdef __cplusplus
}
#endif

#endif  // MOTOTR_CONTROL_H
