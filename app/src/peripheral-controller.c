/**
 * @file peripheral-controller.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Contains all logic concerning peripheral controller objects.
 * @version 0.1
 * @date 2024-11-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "peripheral-controller.h"
#include "libopencm3/cm3/nvic.h"
#include "libopencm3/stm32/f4/adc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/nvic.h"
#include "libopencm3/stm32/f4/usart.h"
#include "uart-control.h"

/* GPIO */

/**
 * @brief function to enable standard gpios.
 *
 * @param periph peripheral to be enabled
 */
static void enableStandardGPIO(PeripheralController *periph)
{
    gpio_mode_setup(periph->peripheral.gpio.port, periph->peripheral.gpio.mode,
                    periph->peripheral.gpio.pupd_resistor, periph->peripheral.gpio.pin);
    periph->status = true;
}

/**
 * @brief function to disable standard gpios. Doesn't actually do anything
 *
 * @param periph peripheral to be disabled.
 */
static void disableStandardGPIO(PeripheralController *periph)
{
    // Placeholder, doesn't actually need to do anything as turning of the RCC
    // surfices
    periph->status = false;
}

/**
 * @brief Create a Standard GPIO object (digital in/out). Nothing fancy.
 *
 * @param port GPIO port
 * @param pin GPIO pin
 * @param clock RCC clock
 * @param input_output is it digital in or out. True = input, false = output
 * @param pupd pullup/pulldown resistor
 * @return PeripheralController
 */
PeripheralController createStandardGPIO(uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                                        PeripheralType input_output, uint8_t pupd)
{
    uint8_t mode = input_output == TYPE_GPIO_INPUT ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT;
    PeripheralController pc;
    pc.type = input_output;
    pc.peripheral.gpio = createGPIOPin(port, pin, clock, mode, 0, pupd);
    pc.enablePeripheral = enableStandardGPIO;
    pc.disablePeripheral = disableStandardGPIO;
    pc.status = false;
    return pc;
}

/**
 * @brief function to enable ADC pins
 *
 * @param periph controller for adc pin
 */
static void enableADCPin(PeripheralController *periph)
{
    gpio_mode_setup(periph->peripheral.adc.port, periph->peripheral.adc.mode, GPIO_PUPD_NONE,
                    periph->peripheral.adc.pin);
    adc_power_off(periph->peripheral.adc.adc_port);
    adc_disable_scan_mode(periph->peripheral.adc.adc_port);
    adc_set_sample_time(periph->peripheral.adc.adc_port, periph->peripheral.adc.adc_channel,
                        periph->peripheral.adc.sample_time);
    adc_power_on(periph->peripheral.adc.adc_port);
    periph->status = true;
}

/**
 * @brief Function to disable adc pins
 *
 * @param periph controller for adc pin
 */
static void disableADCPin(PeripheralController *periph) { periph->status = true; }

/**
 * @brief Function to create ADC peripheral controller
 *
 * @param port Port for physical pin
 * @param pin # of physical pin
 * @param clock clock of adc pin
 * @param sample_time sample time for adc
 * @param adc_port ADC peripheral num, on STM32F411RE its always ADC
 * @param adc_channel ADC channel out of 16.
 * @param adc_clock ADC peripheral clock.
 * @return PeripheralController
 */
PeripheralController createStandardADCPin(uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                                          enum rcc_periph_clken adc_clock, uint32_t sample_time,
                                          uint32_t adc_port, uint8_t adc_channel)
{
    uint8_t              mode = GPIO_MODE_ANALOG;
    PeripheralController pc;
    pc.type = TYPE_ADC;
    pc.peripheral.adc =
        createADCPin(port, pin, clock, adc_clock, sample_time, mode, adc_port, adc_channel);
    pc.enablePeripheral = enableADCPin;
    pc.disablePeripheral = disableADCPin;
    pc.status = false;
    return pc;
}

/**
 * @brief Enable function for UART controllers.
 * 
 * @param periph Peripheral to enable
 */
static void enableUART(PeripheralController *periph)
{
    // Enable both GPIO clocks : might be done elsewhere?
    // rcc_periph_clock_enable(periph.peripheral.uart.TX.clock);
    // rcc_periph_clock_enable(periph.peripheral.uart.RX.clock);

    // config RX
    gpio_mode_setup(periph->peripheral.uart.RX.port, periph->peripheral.uart.RX.mode,
                    periph->peripheral.uart.RX.pupd_resistor, periph->peripheral.uart.RX.pin);
    gpio_set_af(periph->peripheral.uart.RX.port, periph->peripheral.uart.RX.af_mode,
                periph->peripheral.uart.RX.pin); // See datasheet for function.

    // Config TX
    gpio_mode_setup(periph->peripheral.uart.TX.port, periph->peripheral.uart.TX.mode,
                    periph->peripheral.uart.TX.pupd_resistor, periph->peripheral.uart.TX.pin);
    gpio_set_af(periph->peripheral.uart.TX.port, periph->peripheral.uart.TX.af_mode,
                periph->peripheral.uart.TX.pin); // See datasheet for function.

    // Again, clock turned on elsewhere
    // rcc_periph_clock_enable(RCC_USART2);
    usart_set_mode(periph->peripheral.uart.handle,
                   USART_MODE_TX_RX); // Make sure we're Rxing and Txing.

    usart_set_flow_control(periph->peripheral.uart.handle,
                           USART_FLOWCONTROL_NONE); // exclude the rest of RS232.
    usart_set_databits(periph->peripheral.uart.handle, 8);
    usart_set_baudrate(periph->peripheral.uart.handle,
                       periph->peripheral.uart.baudrate); // Set to user defined baudrate.
    usart_set_parity(periph->peripheral.uart.handle, 0);
    usart_set_stopbits(periph->peripheral.uart.handle, 1);

    usart_enable_rx_interrupt(periph->peripheral.uart.handle); // Enable specific Rx interrupt
    nvic_enable_irq(periph->peripheral.uart.nvic_entry);       // Enable interrupts for USART2

    periph->status = true;
    usart_enable(periph->peripheral.uart.handle); // Turn it all on.
}

/**
 * @brief Disable UART peripheral
 * 
 * @param periph peripheral to disable 
 */
static void disableUART(PeripheralController *periph)
{
    usart_disable(periph->peripheral.uart.handle);
    nvic_disable_irq(periph->peripheral.uart.nvic_entry);
    usart_disable_rx_interrupt(periph->peripheral.uart.handle);
    periph->status = false;
}

/**
 * @brief Create a Standard UART/USART object
 * 
 * @param uart_handle handle of target uart
 * @param uart_clock clock of target uart
 * @param baudrate baudrate 
 * @param rx_port recieve port
 * @param tx_port transmit port
 * @param rx_pin recieve pin
 * @param tx_pin transmit pin
 * @param rx_clock recieve clock
 * @param tx_clock transmit clock
 * @param rx_af_mode recieve alternative function mode
 * @param tx_af_mode transmit alternative function mode
 * @param nvic_entry NVIC table entry for UART.
 * @return PeripheralController 
 */
PeripheralController createStandardUARTUSART(uint32_t uart_handle, enum rcc_periph_clken uart_clock,
                                             uint32_t baudrate, uint32_t rx_port, uint32_t tx_port,
                                             uint32_t rx_pin, uint32_t tx_pin,
                                             enum rcc_periph_clken rx_clock,
                                             enum rcc_periph_clken tx_clock, uint8_t rx_af_mode,
                                             uint8_t tx_af_mode, int nvic_entry)
{
    PeripheralController pc;
    pc.type = TYPE_UART;
    pc.peripheral.uart =
        createUARTPeripheral(uart_handle, uart_clock, baudrate, rx_port, tx_port, rx_pin, tx_pin,
                             rx_clock, tx_clock, rx_af_mode, tx_af_mode, nvic_entry);
    pc.enablePeripheral = enableUART;
    pc.disablePeripheral = disableUART;
    pc.status = false;
    return pc;
}