//
// Created by skohl on 14.12.2024.
//modified my T.Weber 18.04.2025
//

#include "induction.h"

void ind_init( INDUCTION_t* inst, TIM_HandleTypeDef* htim) {
    inst->htim = htim;

    // PWM-Frequenz: 11 kHz, Timer-Frequenz: 8 MHz
    uint32_t timer_frequency = 8000000;  // 8 MHz
    uint32_t frequency = 11000;     // wir in main.c auf 11 kHz gesetzt
    uint32_t prescaler = 1;              // Kein Prescaler die PWM frequenz wird direkt mit dem ARR skaliert
    uint32_t arr = (timer_frequency / frequency) - 1; //AutoReload Register wird berechnet -> 726


    // Prescaler und Perioden-Register konfigurieren
    inst->htim->Instance->PSC = prescaler; // Prescaler setzen
    inst->htim->Instance->ARR = arr;       // Auto-Reload-Wert setzen

    // PWM-Kanal starten
    HAL_TIM_PWM_Start(inst->htim, TIM_CHANNEL_3); //  Kanal 3 verwenden
}


void ind_update(INDUCTION_t* inst, uint32_t tick) {
    // Duty Cycle (Tastverhältnis) aktualisieren

    uint32_t duty_cycle = inst->i2c_if.duty_cycle; // Duty Cycle aus der Struktur übernehmen

    // duty_cycle sollte zwischen 0 und 100 (%) liegen
    if (duty_cycle > 100) duty_cycle = 100;
    if (duty_cycle < 0) duty_cycle = 0;

    // Duty Cycle berechnen basierend auf ARR
    duty_cycle = (duty_cycle * (inst->htim->Instance->ARR + 1)) / 100;

    // Vergleichswert (CCR1) setzen
    __HAL_TIM_SET_COMPARE(inst->htim, TIM_CHANNEL_3, duty_cycle);
}
