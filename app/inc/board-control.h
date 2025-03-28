/**
 * @file board-control.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Files containing defines and prototypes for types and functions controlling board IO.
 * @version 0.1
 * @date 2024-11-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef BOARD_CONTROL_H_
#define BOARD_CONTROL_H_

// gcclib includes
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// libopencm3 includes

// local includes
#include "clocks-control.h"
#include "local-memory.h"
#include "peripheral-controller.h"

/**
 * @brief entire board control struct.
 * @param peripherals list of all peripherals enabled.
 * @param clocks list of all clocks (enabled/disabled, static)
 * @param peripherals_count size of peripherals list
 * @param clocks_count size of clocks
 */
typedef struct BoardController
{
    PeripheralController *peripherals;
    ClockController      *clocks;
    size_t                peripherals_count;
    size_t                clocks_count;
    size_t                clocks_size;
    size_t                peripherals_size;
} BoardController;

typedef struct clockExistsReturn
{
    bool exists;
    bool status;
    size_t index; // only if exists
} clockExistsReturn;

// Function Prototypes
BoardController *initBoard(void);
void             deinitBoard(BoardController *bc);
void createDigitalPin(BoardController *bc, uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                      PeripheralType input_output, uint8_t pupd);
uint16_t actionDigitalPin(BoardController *bc, uint32_t port, uint32_t pin, GPIOAction action);
PeripheralType pinExists(BoardController *bc, uint32_t port, uint32_t pin);
void mutateDigitalPin(BoardController *bc, uint32_t port, uint32_t pin, PeripheralType new_type,
                      uint8_t new_pupd);
void mutateADCToDigital(BoardController *bc, uint32_t port, uint32_t pin,
                        enum rcc_periph_clken clock, PeripheralType input_output, uint8_t pupd);
void mutateDigitalToADC(BoardController *bc, uint32_t port, uint32_t pin,
                        enum rcc_periph_clken clock, uint32_t sample_time, uint32_t adc_port,
                        uint8_t adc_channel);
void createAnalogPin(BoardController *bc, uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                     uint32_t sample_time, uint32_t adc_port, uint8_t adc_channel);
uint16_t actionAnalogPin(BoardController *bc, uint32_t port, uint32_t pin);
void createUART(BoardController *bc, uint32_t handle, enum rcc_periph_clken uart_clock,
                uint32_t baudrate, uint32_t rx_port, uint32_t tx_port, uint32_t rx_pin,
                uint32_t tx_pin, enum rcc_periph_clken rx_clock, enum rcc_periph_clken tx_clock,
                uint8_t rx_af_mode, uint8_t tx_af_mode, int nvic_entry);
void killPeripheralOrPin(BoardController *bc, uint32_t port, uint32_t pin);
uint32_t readUARTPort(BoardController *bc, char *data, size_t len);
uint32_t writeUARTPort(BoardController *bc, const char *data, size_t len);


#endif