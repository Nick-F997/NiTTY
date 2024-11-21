/**
 * @file sys_timer.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Definitions for timer logic. Mostly unused.
 * @version 0.1
 * @date 2024-11-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include <stdlib.h>

#include "libopencm3/stm32/timer.h"

typedef struct {
    uint32_t timer;
    enum tim_oc_id channel;
    uint32_t prescaler;
    uint32_t arr_val;
    float duty_cycle;
} PWMPeripheral;

void coreTimerSetup(void);
void corePWMSetDutyCycle(float duty_cycle);


#endif