/**
 * @file gpio-control.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Contains all logic concerning gpio pin controllers (different to peripheralController gpio functions)
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "gpio-control.h"

/**
 * @brief General GPIO setup. Does all the fancy stuff for different peripherals
 * 
 * @param port GPIO port
 * @param pin GPIO pin
 * @param clock RCC clock (can be peripheral clock e.g. RCC_UART2)
 * @param mode input/output/analog/alternative function.
 * @param af_mode alternative function mode, ignore if mode != alternative function.
 * @param pupd_resistor pullup/pulldown resistor.
 * @return GPIOPinController 
 */
GPIOPinController createGPIOPin(uint32_t port, uint32_t pin, 
                        enum rcc_periph_clken clock, uint8_t mode, 
                        uint8_t af_mode, uint8_t pupd_resistor)
{
    return (GPIOPinController){ .port = port, 
                                .pin = pin, 
                                .clock = clock,
                                .mode = mode, 
                                .af_mode = af_mode, 
                                .pupd_resistor = pupd_resistor
                            };
}