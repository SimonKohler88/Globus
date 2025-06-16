//
// Created by skohl on 11.12.2024.
//

#include "motor_control.h"
#include "gpio.h"
#include "i2c.h"

static void set_timer_duty_cycle( MOT_CTRL_t* mot_ctrl, uint32_t value );
static uint32_t get_timer_duty_cycle( MOT_CTRL_t* mot_ctrl );

void mot_ctrl_init( MOT_CTRL_t* mot_ctrl, TIM_HandleTypeDef* htim, uint32_t channel )
{
    mot_ctrl->htim    = htim;
    mot_ctrl->channel = channel;

    /* Timer Input Freq : 8MHz
     * Timer Output Freq: MOT_CTRL_PWM_FREQUENCY (ex: 10000)*/

    int32_t prescale = ( MOT_CTRL_TIMER_INPUT_CLOCK_HZ / ( MOT_CTRL_PWM_FREQUENCY * ( MOT_CTRL_TIMER_RESOLUTION ) ) - 1 );


    if (prescale < 1) prescale = 0;

    htim->Instance->PSC = ( uint32_t ) prescale;
    htim->Instance->ARR = MOT_CTRL_TIMER_RESOLUTION;

    mot_ctrl->last_tick        = 0;
    mot_ctrl->slope_carry_over = 0;

    HAL_TIM_PWM_Start( htim, channel );
}

void mot_ctrl_update( MOT_CTRL_t* mot_ctrl, uint32_t tick )
{
    /* Copy in/out vars */
    i2c_disable_irq();
    MOT_CTRL_I2C_IF_t mot_vars = mot_ctrl->i2c_if;
    i2c_enable_irq();

    uint32_t hw_time                      = tick - mot_ctrl->last_tick;  // ms
    uint32_t current_speed_duty_cycle_tim = get_timer_duty_cycle( mot_ctrl );

    /* check input values */

    if ( mot_vars.target_speed_duty_cycle > 100 ) mot_vars.target_speed_duty_cycle = 100;
    if ( mot_vars.target_speed_duty_cycle < 0 ) mot_vars.target_speed_duty_cycle = 0;
    if ( mot_vars.slope_duty_cycle_per_s > 10000 ) mot_vars.slope_duty_cycle_per_s = 10000;
    if ( mot_vars.slope_duty_cycle_per_s < 1 ) mot_vars.slope_duty_cycle_per_s = 1;

    uint32_t target_speed_duty_cycle_tim = mot_vars.target_speed_duty_cycle * MOT_CTRL_TIMER_RESOLUTION / 100;

    /* check if we have something to do */
    uint8_t must_set = 0;
    if ( current_speed_duty_cycle_tim != target_speed_duty_cycle_tim )
    {
        must_set = 1;
    }
    if ( must_set )
    {
        /*  (slope / 1000/hw time) * 65535/100 = change per hwcycle
         *         ^                   ^
         *  How much change       conversion to
         *  per hw cycle           timer resolution
         *
         *  -> simplifies to:
         */

        float tmp = ( float ) mot_vars.slope_duty_cycle_per_s * ( float ) hw_time * MOT_CTRL_TIMER_RESOLUTION / 100 / 1000 / 2;
        // --> where does the 2 come from? its twice as fast as desired without this...

        /* Keep track of sub-resolution difference */
        float reminder = tmp - ( ( uint32_t ) tmp );

        uint32_t slope_tim_per_hw_cycle = ( uint32_t ) tmp;

        /* take sub-resolution error into account */
        mot_ctrl->slope_carry_over += reminder;
        if ( mot_ctrl->slope_carry_over > 1 )
        {
            slope_tim_per_hw_cycle += 1;
            mot_ctrl->slope_carry_over -= 1;
        }

        /* check in which direction and how much we must change */
        uint8_t current_too_low = 0;
        uint32_t diff           = 0;
        if ( current_speed_duty_cycle_tim < target_speed_duty_cycle_tim )
        {
            current_too_low = 1;
            diff            = target_speed_duty_cycle_tim - mot_vars.current_speed_duty_cycle;
        }
        else
        {
            diff = current_speed_duty_cycle_tim - target_speed_duty_cycle_tim;
        }
        /* convert duty cycle difference (0...100) into timer resolution space (0...65535)  */
        uint32_t diff_tim = diff * MOT_CTRL_TIMER_RESOLUTION / 100;

        /* set as much slope as possible */
        if ( current_too_low )
        {
            if ( diff_tim > slope_tim_per_hw_cycle ) current_speed_duty_cycle_tim += slope_tim_per_hw_cycle;
            else current_speed_duty_cycle_tim += diff_tim;
        }
        else
        {
            if ( diff_tim > slope_tim_per_hw_cycle ) current_speed_duty_cycle_tim -= slope_tim_per_hw_cycle;
            else current_speed_duty_cycle_tim -= diff_tim;
        }
        set_timer_duty_cycle( mot_ctrl, current_speed_duty_cycle_tim );
    }

    /* update i2c if. dont bother of isr ( Atomic? ) */
    mot_ctrl->i2c_if.current_speed_duty_cycle = ( current_speed_duty_cycle_tim * 100 ) / MOT_CTRL_TIMER_RESOLUTION;  // intended non-float div

    mot_ctrl->last_tick = tick;

    /* Brake Pin-> not implemented */
    // TODO: implement Brake Pin
    gpio_set_mot_brake( 1 );
}

static uint32_t get_timer_duty_cycle( MOT_CTRL_t* mot_ctrl )
{
    switch ( mot_ctrl->channel )
    {
        case TIM_CHANNEL_1 : return mot_ctrl->htim->Instance->CCR1;
        case TIM_CHANNEL_2 : return mot_ctrl->htim->Instance->CCR2;
        default : break;
    }
    return 100000;  // unrealistic for tim 15
}

static void set_timer_duty_cycle( MOT_CTRL_t* mot_ctrl, uint32_t value )
{
    uint32_t val_to_set = 0;
    if ( value > MOT_CTRL_TIMER_RESOLUTION ) val_to_set = MOT_CTRL_TIMER_RESOLUTION;
    else val_to_set = value;

    switch ( mot_ctrl->channel )
    {
        case TIM_CHANNEL_1 : mot_ctrl->htim->Instance->CCR1 = val_to_set; break;
        case TIM_CHANNEL_2 : mot_ctrl->htim->Instance->CCR2 = val_to_set; break;
        default : break;
    }
}
