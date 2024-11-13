/**
 * @file local-memory.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Contains helper functions for local memory management.
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "local-memory.h"

/**
 * @brief reallocs memory. 
 * 
 * @param pointer pointer to reallocate
 * @param oldSize old size of pointer
 * @param newSize new size of pointer
 * @return void* realloced mem
 */
void *reallocate(void *pointer, size_t oldSize, size_t newSize)
{
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }

    void *result = realloc(pointer, newSize);
    if (result == NULL) printf("Something has gone wrong:(\r\n");
    return result;
}