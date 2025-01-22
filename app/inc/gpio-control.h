/**
 * @file gpio-control.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Contains GPIO controller structure defines and functions
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef GPIO_CONTROL_H_
#define GPIO_CONTROL_H_

// libgcc includes
#include <stdbool.h>

// libopencm3 includes
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"

// local includes


typedef enum GPIOAction {
    GPIO_READ,
    GPIO_SET,
    GPIO_CLEAR,
    GPIO_TOGGLE,
} GPIOAction;
/**
 * @brief Control structure for GPIO pins. These are not only used as generic GPIO controllers,
 * but also inside other peripheral controllers to indicate what pins are being used for what. 
 * @param port GPIO port
 * @param pin GPIO pin
 * @param clock RCC clock
 * @param mode whether the structure is being used as input, output, analog or alternative function
 * @param af_mode what alternative function mode is used. Ignore if in any other mode.
 * @param pupd_resistor where a pull-up/pull-down resistor is in use. 
 */
typedef struct GPIOPinController {
    uint32_t port;
    uint32_t pin;
    enum rcc_periph_clken clock; // Will sometimes be a peripheral clock
    uint8_t mode; //  input/output/AF/Analog
    uint8_t af_mode; // if mode != AF or ANALOG ingore
    uint8_t pupd_resistor;
} GPIOPinController;

// Function definitions
GPIOPinController createGPIOPin(uint32_t port, uint32_t pin, 
                        enum rcc_periph_clken clock, uint8_t mode, 
                        uint8_t af_mode, uint8_t pupd_resistor);

#endif