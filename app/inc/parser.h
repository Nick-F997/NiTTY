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
/**
 * @brief Enum that defines different operations that can be carried out on pins & peripherals. 
 * 
 */
typedef enum OpCode 
{
    OP_SET,
    OP_RESET,
    OP_TOGGLE,
    OP_READ,
    OP_MAKE_INPUT,
    OP_MAKE_OUTPUT,
} OpCode;
// Struct definitions

// Macro definitions
// Max args for inputOutput function
#define INPUT_OUTPUT_MAX_ARGS (3)

// Size between ports
#define PORT_SIZE (0x400)

// Size to jump between 
#define JUMP_TO_LOWERCASE (0x1B)

// Shit way to make sure a clock is in bounds. 
#define CLOCK_OUT_OF_BOUNDS (RCC_GPIOK)

// Function prototypes
bool parseTokensAndExecute(BoardController *bc, TokenVector *vec);

#endif