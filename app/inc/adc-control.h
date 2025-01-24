/**
 * @file adc-control.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief File contains defines and macros concerning ADC operations in
 * firmware.
 * @version 0.1
 * @date 2025-01-22
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef ADC_CONTROL_H_
#define ADC_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/rcc.h"

typedef struct ADCPinController
{
    uint32_t              port;
    uint32_t              pin;
    enum rcc_periph_clken clock;     // Will sometimes be a peripheral clock
    enum rcc_periph_clken adc_clock; // Should always be ADC1_RCC
    uint16_t              sample_time;
    uint8_t               mode;
    uint32_t              adc_port;
    uint8_t               adc_channel;

} ADCPinController;

ADCPinController createADCPin(uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                              enum rcc_periph_clken adc_clock, uint32_t sample_time, uint8_t mode,
                              uint32_t adc_port, uint8_t adc_channel);

#endif