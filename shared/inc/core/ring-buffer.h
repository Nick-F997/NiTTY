#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include "common-includes.h"

/**
 * @brief Structure for ring buffer. Using binary masking.
 * 
 */
typedef struct ring_buffer_t
{
    uint8_t *buffer;
    uint32_t mask;
    uint32_t read_index;
    uint32_t write_index;
} ring_buffer_t;

void coreRingBufferSetup(ring_buffer_t *rb, uint8_t *buffer, uint32_t size);
bool coreRingBufferEmpty(ring_buffer_t *rb);
bool coreRingBufferWrite(ring_buffer_t *rb, uint8_t byte);
bool coreRingBufferRead(ring_buffer_t *rb, uint8_t *byte);

#endif