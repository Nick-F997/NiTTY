/**
 * @file uart-control.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Defines, function prototypes, and types for uart control.
 * @version 0.1
 * @date 2025-02-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef UART_CONTROL_H_
#define UART_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/ring-buffer.h"

#include "gpio-control.h"
#include "libopencm3/stm32/f4/nvic.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/usart.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/cm3/nvic.h"
#include "libopencm3/stm32/usart.h"

#define RING_BUFFER_SIZE (128)

typedef struct UARTController {
    enum rcc_periph_clken uart_clock;
    uint32_t baudrate;
    GPIOPinController RX;
    GPIOPinController TX;
    uint32_t handle;
    ring_buffer_t rb;
    uint8_t *data_buffer;
    int nvic_entry;
} UARTController;

UARTController createUARTPeripheral(uint32_t uart_handle, enum rcc_periph_clken uart_clock,
                                    uint32_t baudrate, uint32_t rx_port, uint32_t tx_port,
                                    uint32_t rx_pin, uint32_t tx_pin,
                                    enum rcc_periph_clken rx_clock, enum rcc_periph_clken tx_clock,
                                    uint8_t rx_af_mode, uint8_t tx_af_mode, int nvic_entry);
void general_uart_isr(void);
void currentUartWrite(UARTController uart, uint8_t *data, uint32_t len);
void currentUartWriteByte(UARTController uart, uint8_t byte);
uint32_t currentUartRead(UARTController uart, uint8_t *data, uint32_t len);
uint8_t currentUartReadByte(UARTController uart);
bool currentUartDataAvailable(UARTController uart);


#define CREATE_UART_ISR(func_name) \
void func_name(void) {\
do {\
    general_uart_isr(); \
} while (0)\
}
#endif