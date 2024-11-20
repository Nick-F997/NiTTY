/**
 * @file parser.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief header that includes definitions for parser.
 * @version 0.1
 * @date 2024-11-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef PARSER_H_
#define PARSER_H_

// libgcc includes
#include <stdbool.h>


// libopencm3 includes
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"

// local includes
#include "token.h"
#include "board-control.h"

// enum definitions
typedef enum OpCodes {
    OP_RETURN,
    OP_ERROR,
    OP_SETUP_GPIO_INPUT,
    OP_SETUP_GPIO_OUTPUT,
    OP_GPIO_SET,
    OP_GPIO_RESET,
    OP_GPIO_TOGGLE,
    // Add more as we expand functionality
} OpCodes;

// Struct definitions

// Macro definitions
#define INPUT_OUTPUT_MAX_ARGS (3)
#define PORT_SIZE (0x400)
#define JUMP_TO_LOWERCASE (0x1B)

// Function prototypes


#endif