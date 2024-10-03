#ifndef LOCAL_SYSTEM_H_
#define LOCAL_SYSTEM_H_

#include "common-includes.h"

#define CPU_FREQ (84000000)
#define SYSTICK_FREQ (1000)

void coreSystemSetup(void);
uint64_t coreGetTicks(void);
void coreSystemDelay(uint64_t milliseconds);

#endif