/**
 * @file adc-control.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief File containing logic pertaining to ADCPinController
 * @version 0.1
 * @date 2025-01-22
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "adc-control.h"
#include <stdint.h>

ADCPinController createADCPin(uint32_t port, uint32_t pin, enum rcc_periph_clken clock,
                              enum rcc_periph_clken adc_clock, uint32_t sample_time, uint8_t mode,
                              uint32_t adc_port, uint8_t adc_channel) {
    return (ADCPinController){.port = port,
                              .pin = pin,
                              .clock = clock,
                              .sample_time = sample_time,
                              .mode = mode,
                              .adc_port = adc_port,
                              .adc_channel = adc_channel,
                              .adc_clock = adc_clock
                              };
}