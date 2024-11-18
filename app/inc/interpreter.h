/**
 * @file interpreter.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Header containing definitions and prototypes for basic line by line interpreter,.
 * @note much of this relies on work completed by Robert Nystrom in his wonderful work Crafting Interpreters.
 *       Much of this command line interpreter is adapted from the C-Lox interpeter, but with major modifications. 
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
/**
 * @brief Enum that dictates the tokens available to the user.
 * 
 */
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
/**
 * @brief Scanner struct. Scans inputted text and creates tokens.
 * @param start start of current line to be analysed.
 * @param current character from which line is being analysed.
 * 
 */
typedef struct Scanner {
    const char *start;
    const char *current;
} Scanner;

/**
 * @brief Token struct. Emitted by scanner after tokenisation.
 * @param type type of token
 * @param start start of the string for the token
 * @param length length of the string.
 */
typedef struct Token {
    TokenType type;
    const char *start;
    int length;
} Token;


// function prototypes
bool interpret(BoardController *bc, char *source, size_t length);


#endif