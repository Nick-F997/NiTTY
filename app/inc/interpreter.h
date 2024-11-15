/**
 * @file interpreter.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Header containing definitions and prototypes for basic line by line interpreter.
 * @version 0.1
 * @date 2024-11-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef INTERPRETER_H_
#define INTERPRETER_H_

// libgcc includes
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// libopencm3 includes


// local includes
#include "board-control.h"

// Enum defs
typedef enum TokenType {
    TOKEN_GPIO_INPUT,
    TOKEN_GPIO_OUTPUT,
    TOKEN_PORT_PIN,
    TOKEN_GPIO_SET,
    TOKEN_GPIO_RESET,
    TOKEN_GPIO_TOGGLE,
    TOKEN_EOL,
    TOKEN_ERROR,
} TokenType;

// Struct definitions
typedef struct Scanner {
    const char *start;
    const char *current;
} Scanner;

typedef struct Token {
    TokenType type;
    const char *start;
    int length;
} Token;


// function prototypes
bool interpret(BoardController *bc, char *source, size_t length);


#endif