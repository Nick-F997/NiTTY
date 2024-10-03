#include "core/system.h"

#include "libopencm3/stm32/rcc.h"
#include "libopencm3/cm3/systick.h"
#include "libopencm3/cm3/vector.h"

// Only local to this file, but volatile
static volatile uint64_t ticks = 0;

/**
 * @brief Timer interrupt. Increments static variable ticks so we can keep track of time. 
 * 
 */
void sys_tick_handler(void) {
    ticks++;
}

/**
 * @brief Setups up system tick and enables interrupts for 1kHz.
 * 
 */
static void loc_systick_setup(void)
{
    // Could add check here as it returns a bool.
    systick_set_frequency(SYSTICK_FREQ, CPU_FREQ);
    systick_counter_enable();
    systick_interrupt_enable();
}

/**
 * @brief Sets up system clock to 84mHz.
 * 
 */
static void loc_rcc_setup(void)
{
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

/**
 * @brief enables systick and rcc.
 * 
 */
void coreSystemSetup(void)
{
    loc_rcc_setup();
    loc_systick_setup();
}

/**
 * @brief Get time since power on.
 * 
 * @return uint64_t systicks (ms) since power on. 
 */
uint64_t coreGetTicks(void) 
{
    return ticks;
}

