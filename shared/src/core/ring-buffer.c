#include "core/ring-buffer.h"

/**
 * @brief Initialises a ring buffer object.
 * 
 * @param rb the ring buffer to be initialised
 * @param buffer array to initialise ring buffer with
 * @param size size of buffer, must be a power of 2.
 */
void coreRingBufferSetup(ring_buffer_t *rb, uint8_t *buffer, uint32_t size)
{
    rb->buffer = buffer;
    rb->read_index = 0;
    rb->write_index = 0;
    rb->mask = size - 1;
}

/**
 * @brief Tells the user whether the ring buffer has data or not
 * 
 * @param rb ring buffer to check
 * @return true if buffer is empty 
 * @return false if buffer has data
 */
bool coreRingBufferEmpty(ring_buffer_t *rb)
{
    return rb->read_index == rb->write_index;
}

/**
 * @brief Write a byte into a ring buffer.
 * 
 * @param rb buffer to write into
 * @param byte byte to be written
 * @return true if write was successful
 * @return false if write failed
 */
bool coreRingBufferWrite(ring_buffer_t *rb, uint8_t byte)
{
    uint32_t local_write_index = rb->write_index;
    uint32_t local_read_index = rb->read_index;

    // check this. if we write into read_index then we lose all data in the buffer.
    uint32_t next_write_index = (local_write_index + 1) & rb->mask;

    if (next_write_index == local_read_index)
    {
        // drop latest piece of data.
        return false;
    }

    // If we're not at the end increment and update write index.
    rb->buffer[local_write_index] = byte;
    rb->write_index = next_write_index;
    return true;
}

/**
 * @brief Reads a byte out of the ring buffer.
 * 
 * @param rb ring buffer to be read from
 * @param byte pointer to byte to be populated by read.
 * @return true if read was successful
 * @return false if read failed
 */
bool coreRingBufferRead(ring_buffer_t *rb, uint8_t *byte)
{
    // Take local copies as these are available to interrupts etc.
    uint32_t local_read_index = rb->read_index;
    uint32_t local_write_index = rb->write_index;

    // If buffer is full at the start.
    if (local_read_index == local_write_index)
    {
        return false;
    }

    // If it's not, read value, increment read index and return value/success.
    *byte = rb->buffer[local_read_index];
    local_read_index = (local_read_index + 1) & rb->mask; // Masking with power of two wraps read index back around.
    rb->read_index = local_read_index;

    return true;

}
