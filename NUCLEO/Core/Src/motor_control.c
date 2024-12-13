//
// Created by skohl on 11.12.2024.
//

#include "motor_control.h"

void mot_ctrl_init( MOT_CTRL_t* mot_ctrl, TIM_HandleTypeDef* htim ) { mot_ctrl->htim = htim; }
void mot_ctrl_update( MOT_CTRL_t* mot_ctrl )
{
    uint32_t target_speed  = mot_ctrl->target_speed_duty_cycle;
    uint32_t slope         = mot_ctrl->slope_duty_cycle_per_s;

    if ( target_speed > 100 ) target_speed = 100;
    if ( target_speed < 0 ) target_speed = 0;
    if ( slope > 100 ) slope = 100;
    if ( slope < 1 ) slope = 1;

    uint32_t current_speed = ;

    if
}
