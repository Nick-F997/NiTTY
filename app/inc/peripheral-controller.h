/**
 * @file peripheral-controller.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Defines and types of all peripheral controller logic
 * @version 0.1
 * @date 2024-11-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef PERIPHERAL_CONTROLLER_H_
#define PERIPHERAL_CONTROLLER_H_

// gcclib includes
#include <stdlib.h>

// libopencm3 includes
#include "libopencm3/stm32/adc.h"

// local includes
#include "adc-control.h"
#include "gpio-control.h"
#include "uart-control.h"

/**
 * @brief Enum containing all types of peripheral
 *
 */
typedef enum PeripheralType
{
    TYPE_GPIO_INPUT, // Gpio single pin
    TYPE_GPIO_OUTPUT,
    TYPE_UART,
    TYPE_ADC,
    TYPE_OTHER, // Placeholder
    TYPE_NONE,
} PeripheralType;

/**
 * @brief Struct containing individual peripheral information
 * @param type type of peripheral
 * @param peripheral peripheral controller. Pairs with type to create tagged
 * union
 * @param enablePeripheral function pointer to a function that enables this
 * peripheral
 * @param disablePeripheral function pointer to a function that disables this
 * peripheral
 */
typedef struct PeripheralController
{
    PeripheralType type;
    union
    {
        GPIOPinController gpio;
        ADCPinController  adc;
        UARTController uart;
    } peripheral;
    void (*enablePeripheral)(struct PeripheralController *);
    void (*disablePeripheral)(struct PeripheralController *);
    bool status;
} PeripheralController;


// function prototypes
PeripheralController createStandardGPIO(uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                                        PeripheralType input_output, uint8_t pupd);
PeripheralController createStandardADCPin(uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                                          enum rcc_periph_clken adc_clock, uint32_t sample_time,
                                          uint32_t adc_port, uint8_t adc_channel);
PeripheralController createStandardUARTUSART(uint32_t uart_handle, enum rcc_periph_clken uart_clock,
                                             uint32_t baudrate, uint32_t rx_port, uint32_t tx_port,
                                             uint32_t rx_pin, uint32_t tx_pin,
                                             enum rcc_periph_clken rx_clock,
                                             enum rcc_periph_clken tx_clock, uint8_t rx_af_mode,
                                             uint8_t tx_af_mode, int nvic_entry);                                          

#endif