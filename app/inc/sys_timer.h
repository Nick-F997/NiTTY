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