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
#include "libopencm3/stm32/f4/adc.h"
#include "libopencm3/stm32/f4/gpio.h"

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
static void disableADCPin(PeripheralController *periph)
{
    periph->status = true;
}

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
PeripheralController createStandardADCPin(uint32_t port, uint32_t pin, enum rcc_periph_clken clock,enum rcc_periph_clken adc_clock,
                                          uint32_t sample_time, uint32_t adc_port,
                                          uint8_t adc_channel)
{
    uint8_t              mode = GPIO_MODE_ANALOG;
    PeripheralController pc;
    pc.type = TYPE_ADC;
    pc.peripheral.adc = createADCPin(port, pin, clock, adc_clock, sample_time, mode, adc_port, adc_channel);
    pc.enablePeripheral = enableADCPin;
    pc.disablePeripheral = disableADCPin;
    pc.status = false;
    return pc;
}