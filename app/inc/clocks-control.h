/**
 * @file clocks-control.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Header file containing definitions for clockController object and functions
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef CLOCKS_CONTROL_H_
#define CLOCKS_CONTROL_H_

// gcclib includes
#include <stdbool.h>

// libopencm3 includes
#include "libopencm3/stm32/rcc.h"

// local includes

/**
 * @brief ClockController structure
 * @param clock enum for actual peripheral clock
 * @param clock_enabled whether that clock is enabled or not
 */
typedef struct ClockController {
    enum rcc_periph_clken clock;
    bool clock_enabled;
} ClockController;


// Function prototypes. See clocks-control.c for comments.
ClockController create_clock(enum rcc_periph_clken clock_rcc);
void enableClock(ClockController *clock_controller);
void disableClock(ClockController *clock_controller);

#endif