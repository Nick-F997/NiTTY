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
}

/**
 * @brief function to disable standard gpios. Doesn't actually do anything
 * 
 * @param periph peripheral to be disabled.
 */
static void disableStandardGPIO(PeripheralController *periph)
{
    // Placeholder, doesn't actually need to do anything as turning of the RCC surfices
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
PeripheralController createStandardGPIO(uint32_t port, uint32_t pin, enum rcc_periph_clken clock, bool input_output, uint8_t pupd)
{
    uint8_t mode = input_output ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT;
    PeripheralController pc;
    pc.type = TYPE_GPIO;
    pc.peripheral.gpio = createGPIOPin(port, pin, clock, mode, 0, pupd);
    pc.enablePeripheral = enableStandardGPIO;
    pc.disablePeripheral = disableStandardGPIO;
    return pc;
}