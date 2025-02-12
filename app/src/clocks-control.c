/**
 * @file clocks-control.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief All source code dealing with ClockController structure.
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "clocks-control.h"
#include "debug.h"

#ifdef CLOCK_DEBUG
#include <stdio.h>
#endif

/**
 * @brief Create a clock object, disabled by default
 * 
 * @param clock_rcc the rcc_periph_clken enum to be created
 * @return ClockController the created clock object
 */
ClockController create_clock(enum rcc_periph_clken clock_rcc)
{
    return (ClockController) {.clock = clock_rcc, .clock_enabled = false};
}

/**
 * @brief Enables the rcc_peripheral clock
 * 
 * @param clock_controller clock object to be enabled
 */
void enableClock(ClockController *clock_controller)
{
    if (!clock_controller->clock_enabled)
    {
        #ifdef CLOCK_DEBUG
        printf("DEBUG: Clock enabled.\r\n");
        #endif
        rcc_periph_clock_enable(clock_controller->clock);
        clock_controller->clock_enabled = true;
    }
}

/**
 * @brief disables the rcc_peripheral clock
 * 
 * @param clock_controller clock object to be disabled
 */
void disableClock(ClockController *clock_controller)
{
    if (clock_controller->clock_enabled)
    {
        #ifdef CLOCK_DEBUG
        printf("DEBUG: Clock disabled.\r\n");
        #endif
        rcc_periph_clock_disable(clock_controller->clock);
        clock_controller->clock_enabled = false;
    }
}