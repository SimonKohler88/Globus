//
// Created by skohl on 11.12.2024.
//

#include "motor_control.h"


void mot_ctrl_init( MOT_CTRL_t* mot_ctrl, TIM_HandleTypeDef* htim1 ) {mot_ctrl->htim = htim1;}
void mot_ctrl_update( MOT_CTRL_t* mot_ctrl ) {}
void mot_ctrl_set_speed( MOT_CTRL_t* mot_ctrl, float speed ) {}
uint32_t mot_ctrl_get_speed( MOT_CTRL_t* mot_ctrl )
{

}
