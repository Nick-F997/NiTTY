/**
 * @file parser.h
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief header that includes definitions for parser.
 * @version 0.1
 * @date 2024-11-20
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef PARSER_H_
#define PARSER_H_

// libgcc includes
#include <stdbool.h>
#include <stdint.h>

// libopencm3 includes
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/rcc.h"

// local includes
#include "board-control.h"
#include "token.h"

// enum definitions
/**
 * @brief Enum that defines different operations that can be carried out on pins & peripherals.
 *
 */
typedef enum OpCode
{
    OP_SET,
    OP_RESET,
    OP_TOGGLE,
    OP_READ,
    OP_MAKE_INPUT,
    OP_MAKE_OUTPUT,
} OpCode;

// Struct definitions
/**
 * @brief This is the value returned by the function to check if uart pins are valid. 
 * 
 */
typedef struct UARTValidPin {
    bool isValid;
    bool isTx;
    bool isRx;
    uint32_t handle;
    uint8_t af_mode;
} UARTValidPin;


// Macro definitions
// Max args for inputOutput function
#define INPUT_OUTPUT_MAX_ARGS (3)

// ADC defines
#define ADC_MAX_ARGS          (2)
typedef struct {
    uint32_t port;
    uint32_t pin;
    int adc_channel;
} ADCPinMapping;

static const ADCPinMapping adcPinMappings[] = {
    {GPIOA, GPIO0, ADC_CHANNEL0},
    {GPIOA, GPIO1, ADC_CHANNEL1},
    {GPIOA, GPIO4, ADC_CHANNEL4},
    {GPIOA, GPIO5, ADC_CHANNEL5},
    {GPIOA, GPIO6, ADC_CHANNEL6},
    {GPIOA, GPIO7, ADC_CHANNEL7},
    {GPIOB, GPIO0, ADC_CHANNEL8},
    {GPIOB, GPIO1, ADC_CHANNEL9},
    {GPIOC, GPIO0, ADC_CHANNEL10},
    {GPIOC, GPIO1, ADC_CHANNEL11},
    {GPIOC, GPIO2, ADC_CHANNEL12},
    {GPIOC, GPIO3, ADC_CHANNEL13},
    {GPIOC, GPIO4, ADC_CHANNEL14},
    {GPIOC, GPIO5, ADC_CHANNEL15}
 
};

#define ADC_PIN_MAP_SIZE (14)

// defines for UART
#define UART_INIT_MAX_ARGS    (4)
#define UART1_AF GPIO_AF7
#define UART6_AF GPIO_AF8
#define UART1_TX_PIN ((UARTValidPin) {.isValid = true, .handle = USART1, .isTx = true, .isRx = false, .af_mode = UART1_AF})
#define UART1_RX_PIN ((UARTValidPin) {.isValid = true, .handle = USART1, .isTx = false, .isRx = true, .af_mode = UART1_AF})
#define UART6_TX_PIN ((UARTValidPin) {.isValid = true, .handle = USART6, .isTx = true, .isRx = false, .af_mode = UART6_AF})
#define UART6_RX_PIN ((UARTValidPin) {.isValid = true, .handle = USART6, .isTx = false, .isRx = true, .af_mode = UART6_AF})
#define UART_INVALID_PIN ((UARTValidPin){.isValid = false, .handle = 0, .isRx = false, .isTx = false, .af_mode = 0})

#define UART_MAX_READ (32)

// Defines for UART pin mappings
typedef struct {
    uint32_t port;
    uint32_t pin;
    UARTValidPin uartPin;
} UARTPinMapping;

// "lookup  table" for UART pin maps
static const UARTPinMapping uartPinMappings[] = {
    {GPIOA, GPIO9, UART1_TX_PIN},
    {GPIOA, GPIO10, UART1_RX_PIN},
    {GPIOA, GPIO11, UART6_TX_PIN},
    {GPIOA, GPIO12, UART6_RX_PIN},
    {GPIOA, GPIO15, UART1_TX_PIN},
    {GPIOB, GPIO3, UART1_RX_PIN},
    {GPIOB, GPIO6, UART1_TX_PIN},
    {GPIOB, GPIO7, UART1_RX_PIN},
    {GPIOC, GPIO6, UART6_TX_PIN},
    {GPIOC, GPIO7, UART6_RX_PIN}
};

#define UART_PIN_MAP_SIZE (10)

// Size between ports
#define PORT_SIZE (0x400)

// Size to jump between
#define JUMP_TO_LOWERCASE (0x1B)

// Shit way to make sure a clock is in bounds.
#define CLOCK_OUT_OF_BOUNDS (RCC_GPIOK)

#define ADC_OUT_OF_BOUNDS   (ADC_CHANNEL18)

// Function prototypes
bool parseTokensAndExecute(BoardController *bc, TokenVector *vec);

#endif