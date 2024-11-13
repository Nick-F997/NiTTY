/**
 * @file local-memory.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Contains helper macros for local memory allocation and management
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef LOCAL_MEMORY_H_
#define LOCAL_MEMORY_H_

// gcclib includes
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief grows previous capacity
 * 
 */
#define GROW_CAPACITY(cap) ((cap) < 8 ? 8: cap * 2)

/**
 * @brief wrapper around reallocate for ease of use.
 * 
 */
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
    sizeof(type) * (newCount))

/**
 * @brief frees an entire array.
 * 
 */
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * oldCount, 0)

void *reallocate(void *pointer, size_t oldSize, size_t newSize);



#endif