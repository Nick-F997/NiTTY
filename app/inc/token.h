/**
 * @file token.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Include that defines TokenType, Token and TokenVector types and functions relating to them.
 * @version 0.1
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef TOKEN_H_
#define TOKEN_H_

// libgcc includes
#include <stdio.h>
#include <stdlib.h>

// libopencm3 includes

// local includes
#include "local-memory.h"

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
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_EOL,
    TOKEN_ERROR,
} TokenType;

// Struct definitions
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

/**
 * @brief TokenVector structure. A vector of tokens (duh)
 * @param used current used size of vector
 * @param capacity absolute size of vector
 * @param tokens dynamic array of tokens.
 * 
 */
typedef struct TokenVector {
    size_t used;
    size_t capacity;
    Token *tokens;
} TokenVector;

// macro defines

// Function prototypes
TokenVector *initTokenVector(void);
void appendTokenVector(TokenVector *vec, Token tok);
Token getTokenVector(TokenVector *vec, size_t index);
size_t sizeTokenVector(TokenVector *vec);
void deinitTokenVector(TokenVector *vec); 

#endif