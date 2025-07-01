//
// Created by skohl on 14.12.2024.
// modified my T.Weber 18.04.2025
//

#include "induction.h"


uint8_t ind_calculate_deadtime_ticks_from_ns( uint32_t deadtime_ns );
uint32_t ind_calculate_duty_cycle(INDUCTION_t* inst,  uint8_t pwm_duty_cycle_perc);


void ind_init( INDUCTION_t* inst, TIM_HandleTypeDef* htim )
{
    inst->htim = htim;

    uint32_t frequency   = inst->i2c_if.frequency;                    // wir in main.c gesetzt
    uint32_t deadtime_ns = inst->i2c_if.dead_time;                    // Deadtime in Nanosekunden
    uint32_t prescaler   = 0;                                         // Kein Prescaler. die PWM frequenz wird direkt mit dem ARR skaliert
    uint32_t arr         = ( INDUCTION_TIMER_FREQ / frequency ) - 1;  // AutoReload Register wird berechnet

    // Prescaler und Perioden-Register konfigurieren
    htim->Instance->PSC = prescaler;  // Prescaler setzen
    htim->Instance->ARR = arr;        // Auto-Reload-Wert setzen

    // Deadtime konfigurieren (z. B. 100 ns)
    htim->Instance->BDTR |= ( uint32_t ) ind_calculate_deadtime_ticks_from_ns(deadtime_ns);  // Deadtime setzen

    // Duty Cycle berechnen basierend auf ARR
    uint32_t duty_cycle = ind_calculate_duty_cycle(inst, INDUCTION_DUTY_CYCLE_PWM_PERC);
    // Vergleichswert (CCR1) setzen
    __HAL_TIM_SET_COMPARE( inst->htim, TIM_CHANNEL_3, duty_cycle );

    // Komplementärsignal aktivieren
    htim->Instance->CCER |= TIM_CCER_CC3NE;

    htim->Instance->BDTR |= TIM_BDTR_MOE;  // Hauptausgabe aktivieren (Main Output Enable)

    // PWM-Kanal starten
    HAL_TIM_PWM_Start( inst->htim, TIM_CHANNEL_3 );  //  Kanal 3 verwenden
}

void ind_update( INDUCTION_t* inst, uint32_t tick )
{
    // nix zu tun hier.

    // falls i2c zukommen sollte, muss hier abgehandelt werden
}

uint32_t ind_calculate_duty_cycle(INDUCTION_t* inst,  uint8_t pwm_duty_cycle_perc)
{
    return (  pwm_duty_cycle_perc* ( inst->htim->Instance->ARR + 1 ) ) / 100;
}

uint8_t ind_calculate_deadtime_ticks_from_ns( uint32_t deadtime_ns )
{
    // Zur sicherheit mindestens 500 ns! Ansonsten Kurzschluss in Halbbrücke
    if ( deadtime_ns < INDUCTION_MINIMUM_DEAD_TIME_NS ) deadtime_ns = INDUCTION_MINIMUM_DEAD_TIME_NS;

    uint32_t deadtime_ticks = ( ( float ) INDUCTION_TIMER_FREQ / 1000000000 ) * deadtime_ns;  // Berechnung in Takten
    if ( deadtime_ticks > 0xFF ) deadtime_ticks = 0xFF;                                       // Maximalwert begrenzen (8-Bit)

    return ( uint8_t ) deadtime_ticks;
}