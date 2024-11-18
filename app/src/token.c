/**
 * @file token.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief File containing logic for Token, TokenType, and TokenVector.
 * @version 0.1
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "token.h"

/**
 * @brief Function that initialises a token vector.
 * 
 * @return TokenVector* initialised token vector.
 */
TokenVector *initTokenVector(void)
{
    TokenVector *vec = (TokenVector *)malloc(sizeof(TokenVector));
    vec->capacity = 4;
    vec->used = 0;
    vec->tokens = (Token *)malloc(sizeof(Token) * vec->capacity);
    return vec;
}

/**
 * @brief function to deinit token vector
 * 
 * @param vec vector to be deinit.
 */
void deinitTokenVector(TokenVector *vec)
{
    free(vec->tokens);
    vec->capacity = 0;
    vec->used = 0;
    free(vec);
}

/**
 * @brief Function to append to the tail of a token vector
 * 
 * @param vec vector to be appended to
 * @param tok token to append
 */
void appendTokenVector(TokenVector *vec, Token tok)
{
    if (vec->used == vec->capacity)
    {
        size_t oldsize = vec->capacity;
        vec->capacity = GROW_CAPACITY(oldsize);
        vec->tokens = GROW_ARRAY(Token, vec->tokens, oldsize, vec->capacity);
    }

    vec->tokens[vec->used++] = tok;
}

/**
 * @brief Returns the size of a token vector object
 * 
 * @param vec object to return size of
 * @return size_t size of vector
 */
size_t sizeTokenVector(TokenVector *vec)
{
    return vec->used;
}

/**
 * @brief Get an element from TokenVector object
 * 
 * @param vec vector to get element from
 * @param index index to get element from
 */
Token getTokenVector(TokenVector *vec, size_t index)
{
    if (index >= vec->used)
    {
        // TODO: This is stupid, I'll come up with a better option.
        while (1)
        {
            printf("Element out of range! Index %d\r\n", index);
        }
    }

    return vec->tokens[index];
}