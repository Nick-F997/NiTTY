#include "libopencm3/cm3/nvic.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/usart.h"

#include "core/uart.h"

static uint8_t data_buffer = 0U;
static bool data_available = false;

/**
 * @brief interrupt service routine for UART RX.
 * 
 */
void usart2_isr(void)
{
    const bool overrun_occured = usart_get_flag(USART2, USART_FLAG_ORE /* Overrun flag? */) == 1;
    const bool received_data = usart_get_flag(USART2, USART_FLAG_RXNE /* Recieved data? */) == 1;

    if (overrun_occured || received_data)
    {
        data_buffer = (uint8_t)usart_recv(USART2);
        data_available = true;
    }
}

/**
 * @brief Sets up USART2 for PC-MCU communications
 * 
 * @param baudrate the desired baudrate.
 */
void coreUartSetup(uint32_t baudrate)
{
    rcc_periph_clock_enable(RCC_USART2);
    usart_set_mode(USART2, USART_MODE_TX_RX); // Make sure we're Rxing and Txing.

    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE); // exclude the rest of RS232.
    usart_set_databits(USART2, 8);
    usart_set_baudrate(USART2, baudrate); // Set to user defined baudrate.
    usart_set_parity(USART2, 0);
    usart_set_stopbits(USART2, 1);

    usart_enable_rx_interrupt(USART2); // Enable specific Rx interrupt
    nvic_enable_irq(NVIC_USART2_IRQ);  // Enable interrupts for USART2

    usart_enable(USART2); // Turn it all on.
}

/**
 * @brief Writes a buffer of length `len` to USART2.
 * 
 * @param data buffer to be written
 * @param len length of data.
 */
void coreUartWrite(uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        coreUartWriteByte(data[i]);
    }
}

/**
 * @brief Writes an individual byte to USART2.
 * 
 * @param byte byte to be written.
 */
void coreUartWriteByte(uint8_t byte)
{
    usart_send_blocking(USART2, (uint16_t)byte);
}

/**
 * @brief Reads `len` bytes into data if data is available in UART module.
 * 
 * @param data buffer to be written into
 * @param len length to read.
 * @return uint32_t the amount of bytes actually read. If it != len then something's gone wrong.
 */
uint32_t coreUartRead(uint8_t *data, uint32_t len)
{
    if (len > 0 && data_available) 
    {
        *data = data_buffer;
        data_available = false;
        return 1;
    }

    return 0;
}

/**
 * @brief Reads a single byte from USART2.
 * 
 * @return uint8_t byte read.
 */
uint8_t coreUartReadByte(void)
{
    data_available = false;
    return data_buffer;
}

/**
 * @brief Gets data_available variable.
 * 
 * @return true if USART buffer has data
 * @return false if USART buffer does not have data.
 */
bool coreUartDataAvailable(void)
{
    return data_available;
}