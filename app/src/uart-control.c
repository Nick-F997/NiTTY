/**
 * @file uart-control.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Contains all logic for low level UART control.
 * @version 0.1
 * @date 2025-02-03
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "uart-control.h"
#include "libopencm3/stm32/f4/nvic.h"

static UARTController currently_active_uart = {0U};

/**
 * @brief Low level function to return UART controller
 *
 * @param uart_handle handle for UART (e.g. USART2)
 * @param uart_clock clock for UART
 * @param baudrate baudrate for UART
 * @param rx_port recieve port identifier
 * @param tx_port transmit port identifier
 * @param rx_pin recieve pin identifier
 * @param tx_pin transmit port identifier
 * @param rx_clock recieve gpio clock
 * @param tx_clock transmit gpio clock
 * @param rx_af_mode recieve alternative function mode
 * @param tx_af_mode transmit alternative function mode
 * @param nvic_entry interrupt table isr for this peripheral.
 * @return UARTController
 */
UARTController createUARTPeripheral(uint32_t uart_handle, enum rcc_periph_clken uart_clock,
                                    uint32_t baudrate, uint32_t rx_port, uint32_t tx_port,
                                    uint32_t rx_pin, uint32_t tx_pin,
                                    enum rcc_periph_clken rx_clock, enum rcc_periph_clken tx_clock,
                                    uint8_t rx_af_mode, uint8_t tx_af_mode, int nvic_entry)
{
    uint8_t           mode = GPIO_MODE_AF;
    GPIOPinController rx =
        createGPIOPin(rx_port, rx_pin, rx_clock, mode, rx_af_mode, GPIO_PUPD_NONE);
    GPIOPinController tx =
        createGPIOPin(tx_port, tx_pin, tx_clock, mode, tx_af_mode, GPIO_PUPD_NONE);

    ring_buffer_t rb;
    uint8_t       data_buffer[RING_BUFFER_SIZE] = {0U};
    coreRingBufferSetup(&rb, data_buffer, RING_BUFFER_SIZE);

    UARTController uart = {
        .handle = uart_handle,
        .uart_clock = uart_clock,
        .baudrate = baudrate,
        .nvic_entry = nvic_entry,
        .RX = rx,
        .TX = tx,
        .data_buffer = data_buffer,
        .rb = rb,
    };

    // Just a copy rather than a reference is needed, i think
    currently_active_uart = uart;
    return uart;
}

/**
 * @brief Define all isrs. Most should be disabled when not in use.
 * 
 */
void usart1_isr(void) { general_uart_isr(); }
void usart3_isr(void) { general_uart_isr(); }
void uart4_isr(void) { general_uart_isr(); }
void uart5_isr(void) { general_uart_isr(); }
void usart6_isr(void) { general_uart_isr(); }
void uart7_isr(void) { general_uart_isr(); }
void uart8_isr(void) { general_uart_isr(); }

/**
 * @brief General UART interrupt service routine for the currently active UART controller.
 *
 */
void general_uart_isr(void)
{
    const bool overrun_occured =
        usart_get_flag(currently_active_uart.handle, USART_FLAG_ORE /* Overrun flag? */) == 1;
    const bool received_data =
        usart_get_flag(currently_active_uart.handle, USART_FLAG_RXNE /* Recieved data? */) == 1;

    if (overrun_occured || received_data)
    {
        if (!coreRingBufferWrite(&currently_active_uart.rb,
                                 (uint8_t)usart_recv(currently_active_uart.handle)))
        {
            // Handle failure. Update buffer size.
        }
    }
}

/**
 * @brief Writes a buffer of length `len` to USART2.
 *
 * @param data buffer to be written
 * @param len length of data.
 */
void currentUartWrite(UARTController uart, uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        currentUartWriteByte(uart, data[i]);
    }
}

/**
 * @brief Writes an individual byte to USART2.
 *
 * @param byte byte to be written.
 */
void currentUartWriteByte(UARTController uart, uint8_t byte)
{
    usart_send_blocking(uart.handle, (uint16_t)byte);
}

/**
 * @brief Reads `len` bytes into data if data is available in UART module.
 *
 * @param data buffer to be written into
 * @param len length to read.
 * @return uint32_t the amount of bytes actually read. If it != len then something's gone wrong.
 */
uint32_t currentUartRead(UARTController uart, uint8_t *data, uint32_t len)
{
    // We need to read more than 0 bytes.
    if (len <= 0)
    {
        return 0;
    }

    for (uint32_t num_bytes = 0; num_bytes < len; num_bytes++)
    {
        if (!coreRingBufferRead(&uart.rb, &data[num_bytes]))
        {
            return num_bytes; // num_bytes is how many bytes we've read.
        }
    }

    return len; // we read all the bytes
}

/**
 * @brief Reads a single byte from USART2. User responsibility to check data is available.
 *
 * @return uint8_t byte read.
 */
uint8_t currentUartReadByte(UARTController uart)
{
    uint8_t byte;
    (void)currentUartRead(uart, &byte, 1); // cast it to void cus we don't care if it fails.
    return byte;
}

/**
 * @brief Gets data_available variable.
 *
 * @return true if USART buffer has data
 * @return false if USART buffer does not have data.
 */
bool currentUartDataAvailable(UARTController uart) { return !coreRingBufferEmpty(&uart.rb); }