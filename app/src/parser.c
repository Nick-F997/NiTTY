/**
 * @file parser.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Contains logic for parsing and executing token vectors.
 * @version 0.1
 * @date 2024-11-20
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "parser.h"
#include "board-control.h"
#include "libopencm3/stm32/f4/adc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/nvic.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/usart.h"
#include "peripheral-controller.h"
#include "token.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Get the Clock that matches the port provided.
 *        This could be done with maths but honestly, this is far simpler.
 * @param port port to get clock of
 * @return enum rcc_periph_clken
 */
static enum rcc_periph_clken getClockFromPort(uint32_t port)
{
    switch (port)
    {
    case GPIOA:
    {
        return RCC_GPIOA;
    }
    case GPIOB:
    {
        return RCC_GPIOB;
    }
    case GPIOC:
    {
        return RCC_GPIOC;
    }
    case GPIOD:
    {
        return RCC_GPIOD;
    }
    case GPIOE:
    {
        return RCC_GPIOE;
    }
    default:
    {
        // Bit of a crap way of doing this, but this clock is out of bounds.
        return CLOCK_OUT_OF_BOUNDS;
    }
    }
}

/**
 * @brief Parses a GPIO pin from a given token. Some notes on execution (also
 * covered in code below): Ports are calculated as some offset from the
 * peripheral base. For example, GPIOA is
 *        (((0x40000000U) + 0x20000) + 0x0000), where 0x0000 is the offset. The
 * offset value is 0x400, Which is saved as PORT_SIZE. We multiply this by the
 * current character minus the first character (subtracter). Each pin is indexed
 * in a port by a bit shift along. E.g. GPIO0 = (1 << 0), GPIO1 = (1 << 1),
 * GPIO2 = (1 << 2), etc.
 *
 * @param token Token to be parsed.
 * @param port Pointer to uint32_t value where selected port is stored
 * @param pin Pointer to uint32_t value where selected pin is stored
 * @return true parsed both values
 * @return false one or both parsings failed.
 */
static bool parsePortPin(Token token, uint32_t *port, uint32_t *pin)
{
    // values that indicate failure.
    bool parsed_port = false;
    bool parsed_pin = false;

    // Variable that modifies what value we're using.
    uint32_t subtractor = (uint32_t)PORTA_STM32F411RE;

    // Initialise port to first port
    *port = GPIOA;

    // Loop over all possible port characters
    for (char port_char = PORTA_STM32F411RE; port_char < (PORTe_STM32F411RE + 1); port_char++)
    {
        // Check port character
        if (token.start[0] == port_char)
        {
            // Ports are calculated as some offset from the peripheral base. For
            // example, GPIOA is
            // (((0x40000000U) + 0x20000) + 0x0000), where 0x0000 is the offset.
            // The offset value is 0x400, Which is saved as PORT_SIZE. We
            // multiply this by the current character minus the first character
            // (subtracter).
            *port += PORT_SIZE * ((uint32_t)port_char - subtractor);
            parsed_port = true;
            break;
        }
        // If we hit 'E', we jump to 'a'. We then update subtractor to
        // essentially loop back to the start, as a10 == A10.
        if (port_char == PORTE_STM32F411RE)
        {
            port_char += JUMP_TO_LOWERCASE;
            subtractor = (uint32_t)PORTa_STM32F411RE;
        }
    }

    // Increment away from the port identifier
    token.start++;
    // Parse remaining value.
    // Error checking could be done here but as mentioned it should already be
    // correct.
    uint32_t pin_val = strtoul(token.start, NULL, 10);

    // This should never be outside of this range as it's checked in the scanner
    // but better safe than sorry.
    if (pin_val <= 15)
    {
        // Calculate pin value mathematically
        // Each pin is indexed in a port by a bit shift along. E.g. GPIO0 = (1
        // << 0), GPIO1 = (1 << 1), GPIO2 = (1 << 2), etc.
        *pin = (1 << pin_val);
        parsed_pin = true;
    }
    return parsed_port && parsed_pin; // Return false if either parse failed.
}

/**
 * @brief Returns ADC1. Other hardware will need to implement a whole shebang for this
 *
 * @return uint32_t ADC base channel
 */
static uint32_t getADCBase(void) { return ADC1; }

/**
 * @brief Returns the channel from the port/pin identifier. Developed based on stm32-nucleoF411RE
 * datasheets.
 *
 * @param port port to identify
 * @param pin pin to identify
 * @return int The ADC channel
 */
static int getADCChannelFromPortPin(uint32_t port, uint32_t pin)
{
    for (size_t i = 0; i < ADC_PIN_MAP_SIZE; i++)
    {
        if (adcPinMappings[i].port == port && adcPinMappings[i].pin == pin)
        {
            return adcPinMappings[i].adc_channel;
        }
    }
    printf("Error: This pin is not usable for ADC.\r\n");
    return ADC_OUT_OF_BOUNDS;
}

/**
 * @brief Creates an input or output object in board controller based on input.
 *
 * @param bc board controller object to create input/output on.
 * @param vec vector of tokens to parse
 * @param input_output is it input or output.
 * @return true parsed successfully.
 * @return false parsed unsuccessfully.
 */
static bool inputOutput(BoardController *bc, TokenVector *vec, OpCode input_output)
{
    size_t vec_size = sizeTokenVector(vec);
    // Ignore end of line token.
    if (vec_size - 1 != INPUT_OUTPUT_MAX_ARGS)
    {
        printf("> Parse Error: Invalid input format, use \"input <port pin> "
               "<pup/pdown/none>\". See documentation for more information.\r\n");
        return false;
    }

    // init empty vars, we check for null later to make sure all were assigned.
    uint8_t  pupd = 0;
    uint32_t port = 0;
    uint32_t pin = 0;
    bool     port_pin_set = false;
    bool     pupd_set = false;

    // Ignore the initial token - we've already parsed this.
    for (size_t i = 1; i < vec_size; i++)
    {
        Token current_token = getTokenVector(vec, i);
        switch (current_token.type)
        {
        case TOKEN_EOL:
        {
            // Just ignore.
            break;
        }
        case TOKEN_PORT_PIN:
        {
            // If we haven't already assigned the port and pin
            if (!port_pin_set)
            {
                if (!parsePortPin(current_token, &port, &pin))
                {
                    printf("> Parse Error: Unable to parse GPIO identifer "
                           "\"%.*s\".",
                           current_token.length, current_token.start);
                    return false;
                }
                port_pin_set = true;
            }
            else
            {
                printf("> Parse Error: Multiple GPIO pins provided.\r\n");
                return false;
            }

            break;
        }
        // These all do the same thing
        case TOKEN_GPIO_NORESISTOR:
        case TOKEN_GPIO_PULLUP:
        case TOKEN_GPIO_PULLDOWN:
        {
            // if we haven't already set pupd
            if (!pupd_set)
            {
                pupd = (current_token.type == TOKEN_GPIO_NORESISTOR) ? GPIO_PUPD_NONE
                       : (current_token.type == TOKEN_GPIO_PULLUP)   ? GPIO_PUPD_PULLUP
                                                                     : GPIO_PUPD_PULLDOWN;
                pupd_set = true;
            }
            else
            {
                printf("> Parse Error: Multiple pullup/pulldown resistor "
                       "configurations provided.\r\n");
                return false;
            }
            break;
        }
        default:
        {
            printf("> Parse Error: Unrecognised token while parsing: "
                   "\"%.*s\".\r\n",
                   current_token.length, current_token.start);
            return false;
        }
        }
    }

    // If we've gotten this far we know we've succeeded.
    if (port_pin_set && pupd_set)
    {
        // Translate opcode into peripheral type.
        PeripheralType type_input_output =
            input_output == OP_MAKE_INPUT ? TYPE_GPIO_INPUT : TYPE_GPIO_OUTPUT;
        // If the pin doesn't exist make a new one, else just change the current
        // one.
        PeripheralType pin_exists = pinExists(bc, port, pin);
        if (pin_exists == TYPE_NONE)
        {
            // Get correct clock
            enum rcc_periph_clken clock = getClockFromPort(port);
            if (clock == CLOCK_OUT_OF_BOUNDS)
            {
                // This really shouldn't happen.
                printf("> Parse Error: Incorrect port clock identified.\r\n");
                return false;
            }
            createDigitalPin(bc, port, pin, clock, type_input_output, pupd);
            printf("> created new pin.\r\n");
        }
        else if (pin_exists == TYPE_GPIO_INPUT || pin_exists == TYPE_GPIO_OUTPUT)
        {
            // This pin exists so just update it.
            mutateDigitalPin(bc, port, pin, type_input_output, pupd);
            printf("> modified existing pin.\r\n");
        }
        else
        {
            switch (pin_exists)
            {
            case TYPE_ADC:
            {
                // Don't need to check if clock is valid because it should be already
                enum rcc_periph_clken clock = getClockFromPort(port);
                mutateADCToDigital(bc, port, pin, clock, type_input_output, pupd);
                printf("> Modified ADC to GPIO pin.\r\n");
                break;
            }
            case TYPE_UART:
            {
                // Kill entire UART peripheral
                printf("> Warning: Disabling entire UART port to convert to GPIO...\r\n");
                killPeripheralOrPin(bc, port, pin);
                // Don't need to check if clock is valid because it should be already
                enum rcc_periph_clken clock = getClockFromPort(port);
                // Recreate GPIO from scratch.
                createDigitalPin(bc, port, pin, clock, type_input_output, pupd);
                printf("> Modified UART to GPIO pin.\r\n");
                break;
            }
            default:
            {
                printf("> Failed to modify pin. You shouldn't have ended up here!\r\n");
                return false;
            }
            }
        }
        return true;
    }
    else
    {
        printf("> Parse Error: Unable to parse identifiers.\r\n");
        return false;
    }
}

/**
 * @brief Set, reset, read, or toggle the selected pin(s).
 *
 * @param bc board control object
 * @param vec vector of tokens
 * @param operation What operation the user has selected
 * @return true executed correctly
 * @return false executed incorrectly.
 */
static bool setResetToggleRead(BoardController *bc, TokenVector *vec, OpCode operation)
{
    // We don't need to do a check on this as we allow users to input as many
    // pins as they want.
    size_t vec_size = sizeTokenVector(vec);

    // initialise to 0
    uint32_t port = 0;
    uint32_t pin = 0;

    // Loop through vector, ignoring first token.
    for (size_t i = 1; i < vec_size; i++)
    {
        Token    current_token = getTokenVector(vec, i);
        uint16_t read_response = 0;          // response if the user wants to read the
                                             // pin. Will be zero if the pin is output.
        if (current_token.type == TOKEN_EOL) // Ignore EOL token.
        {
            continue;
        }
        else if (current_token.type == TOKEN_PORT_PIN)
        {
            // If we can't parse the object return
            if (!parsePortPin(current_token, &port, &pin))
            {
                printf("> Parse Error: Unrecognised port pin: \"%.*s\".\r\n", current_token.length,
                       current_token.start);
                return false;
            }

            // If the pin already exists execute command.
            if (pinExists(bc, port, pin) == TYPE_GPIO_INPUT ||
                pinExists(bc, port, pin) == TYPE_GPIO_OUTPUT)
            {
                switch (operation)
                {
                case OP_SET:
                {
                    read_response = actionDigitalPin(bc, port, pin, GPIO_SET);
                    printf("> SET %.*s\r\n", current_token.length, current_token.start);
                    break;
                }
                case OP_RESET:
                {
                    read_response = actionDigitalPin(bc, port, pin, GPIO_CLEAR);
                    printf("> RESET %.*s\r\n", current_token.length, current_token.start);
                    break;
                }
                case OP_TOGGLE:
                {
                    read_response = actionDigitalPin(bc, port, pin, GPIO_TOGGLE);
                    printf("> TOGGLE %.*s\r\n", current_token.length, current_token.start);
                    break;
                }
                case OP_READ:
                {
                    read_response = actionDigitalPin(bc, port, pin, GPIO_READ);
                    printf("> READ %.*s = %u\r\n", current_token.length, current_token.start,
                           read_response);
                    break;
                }
                default:
                {
                    // Should never get here.
                    printf("> Parse Error: Incorrect op code provided.\r\n");
                    return false;
                }
                }
            }
            else if (pinExists(bc, port, pin) == TYPE_ADC)
            {
                switch (operation)
                {
                case OP_READ:
                {
                    read_response = actionAnalogPin(bc, port, pin);
                    printf("> READ %.*s (ADC) = %u\r\n", current_token.length, current_token.start,
                           read_response);
                    break;
                }
                default:
                {
                    printf("> Parse Error: this operation is unavailable for this pin "
                           "configuration (ADC).\r\n");
                    return false;
                }
                }
            }
            else
            {
                // This pin does not exist, stop execution.
                printf("> Parse Error: Port Pin identifer \"%.*s\" is not "
                       "initialised and cannot be operated on.\r\n",
                       current_token.length, current_token.start);
                return false;
            }
            // Reset port and pin for safety.
            port = pin = 0;
        }
        else
        {
            printf("> Parse Error: Unrecognised token while parsing: "
                   "\"%.*s\".\r\n",
                   current_token.length, current_token.start);
            return false;
        }
    }

    // Execution was succesful.
    return true;
}

/**
 * @brief Function for defining ADC peripherals.
 *
 * @param bc Board Controller object
 * @param vec vector of tokens
 * @return true ADC successfully initialised
 * @return false ADC not initialised
 */
static bool adc(BoardController *bc, TokenVector *vec)
{
    size_t vec_size = sizeTokenVector(vec);
    // Ignore end of line token.
    if (vec_size - 1 != ADC_MAX_ARGS)
    {
        printf("> Parse Error: Invalid input format, use \"adc <port pin>\". See documentation for "
               "more information.\r\n");
        return false;
    }

    uint32_t port = 0;
    uint32_t pin = 0;
    bool     port_pin_set = false;

    // Ignore the initial token - we've already parsed this.
    for (size_t i = 1; i < vec_size; i++)
    {
        Token current_token = getTokenVector(vec, i);
        switch (current_token.type)
        {
        case TOKEN_EOL:
        {
            // ignore
            break;
        }
        case TOKEN_PORT_PIN:
        {
            // If we haven't already assigned the port and pin
            if (!port_pin_set)
            {
                if (!parsePortPin(current_token, &port, &pin))
                {
                    printf("> Parse Error: Unable to parse ADC identifer "
                           "\"%.*s\".",
                           current_token.length, current_token.start);
                    return false;
                }
                port_pin_set = true;
            }
            else
            {
                printf("> Parse Error: Multiple ADC pins provided.\r\n");
                return false;
            }

            break;
        }
        default:
        {
            printf("> Parse Error: Unrecognised token while parsing: "
                   "\"%.*s\".\r\n",
                   current_token.length, current_token.start);
            return false;
        }
        }
    }
    if (port_pin_set)
    {
        // If the pin already exists... See if we can mutate it.
        PeripheralType pin_exists = pinExists(bc, port, pin);
        switch (pin_exists)
        {
        case TYPE_GPIO_INPUT:
        case TYPE_GPIO_OUTPUT:
        {
            enum rcc_periph_clken clock = getClockFromPort(port);
            uint32_t              base = getADCBase();
            int                   channel = getADCChannelFromPortPin(port, pin);
            int                   sample_time = ADC_SMPR_SMP_3CYC;
            if (channel == ADC_OUT_OF_BOUNDS)
            {
                printf("> Error: Pin is not available for use as ADC.\r\n");
                return false;
            }
            mutateDigitalToADC(bc, port, pin, clock, sample_time, base, channel);
            printf("> Modified GPIO to ADC pin.\r\n");
            break;
        }
        case TYPE_UART:
        {
            // Only difference with UART is the whole peripheral gets turned off.

            // Turn off the whole UART port
            printf("> Warning: Disabling entire UART port to convert to ADC...\r\n");
            killPeripheralOrPin(bc, port, pin);
            enum rcc_periph_clken clock = getClockFromPort(port);
            uint32_t              base = getADCBase();
            int                   channel = getADCChannelFromPortPin(port, pin);
            int                   sample_time = ADC_SMPR_SMP_3CYC;
            if (channel == ADC_OUT_OF_BOUNDS)
            {
                printf("> Error: Pin is not available for use as ADC.\r\n");
                return false;
            }
            createAnalogPin(bc, port, pin, clock, sample_time, base, channel);
            printf("> created new ADC pin.\r\n");
            break;
        }
        case TYPE_ADC:
        {
            return true; // Do nothing.
        }
        case TYPE_NONE:
        {
            enum rcc_periph_clken clock = getClockFromPort(port);
            uint32_t              base = getADCBase();
            int                   channel = getADCChannelFromPortPin(port, pin);
            int                   sample_time = ADC_SMPR_SMP_3CYC;
            if (channel == ADC_OUT_OF_BOUNDS)
            {
                printf("> Error: Pin is not available for use as ADC.\r\n");
                return false;
            }
            createAnalogPin(bc, port, pin, clock, sample_time, base, channel);
            printf("> created new ADC pin.\r\n");
            break;
        }
        default:
        {
            printf("Not implemented yet!\r\n");
            return false;
        }
        }
        return true;
    }
    else
    {
        printf("> Parse Error: Unrecognised pin.\r\n");
        return false;
    }
}

/**
 * @brief Returns the uart handle for the provided pin. This could be done more effectively with a
 * lookup table, but for time's sakes I'm doing it dumb.
 *
 * @param port to check
 * @param pin to check
 * @return UARTValidPin a struct containing info about that pin's possible config.
 */
static UARTValidPin getUARTInfo(uint32_t port, uint32_t pin)
{
    for (size_t i = 0; i < UART_PIN_MAP_SIZE; i++)
    {
        if (uartPinMappings[i].port == port && uartPinMappings[i].pin == pin)
        {
            return uartPinMappings[i].uartPin;
        }
    }
    return UART_INVALID_PIN;
}

/**
 * @brief Returns the clock corresponding to the uart handle
 *
 * @param handle handle to get clock for
 * @return enum rcc_periph_clken identified
 */
static enum rcc_periph_clken getUARTclock(uint32_t handle)
{
    switch (handle)
    {
    case USART1:
    {
        return RCC_USART1;
    }
    case USART6:
    {
        return RCC_USART6;
    }
    default:
    {
        return CLOCK_OUT_OF_BOUNDS;
    }
    }
}

/**
 * @brief Gets the NVIC table entry for the specified UART
 * 
 * @param handle UART handle
 * @return int NVIC table entry
 */
static int getUARTNVICEntry(uint32_t handle)
{
    switch (handle)
    {
    case USART1:
    {
        return NVIC_USART1_IRQ;
    }
    case USART6:
    {
        return NVIC_USART6_IRQ;
    }
    default:
    {
        return 0;
    }
    }
}

/**
 * @brief Parse a UART command and initialises a UART peripheral from scratch.
 * 
 * @param bc board controller object
 * @param vec vector of tokens
 * @return true set up successfully
 * @return false set up unsuccessful 
 */
static bool uartInitialise(BoardController *bc, TokenVector *vec)
{
    size_t vec_size = sizeTokenVector(vec);
    if (vec_size - 1 != UART_INIT_MAX_ARGS)
    {
        printf("> Parse Error: Invalid input format, use \"adc <port pin>\". See documentation for "
               "more information.\r\n");
        return false;
    }

    // Define different variables.
    uint32_t rx_port = 0;
    uint32_t rx_pin = 0;
    uint32_t tx_port = 0;
    uint32_t tx_pin = 0;
    uint32_t baud_rate = 0;
    bool     rx_port_pin_set = false;
    bool     tx_port_pin_set = false;
    bool     baud_rate_set = false;

    for (size_t i = 1; i < vec_size; i++)
    {
        Token current_token = getTokenVector(vec, i);
        switch (current_token.type)
        {
        case TOKEN_PORT_PIN:
        {
            // UART needs 2 pins to create. This first statment checks and creates RX pin
            if (!rx_port_pin_set && !tx_port_pin_set)
            {
                if (!parsePortPin(current_token, &rx_port, &rx_pin))
                {
                    printf("Error: Unable to parse UART RX pin \"%.*s\".\r\n", current_token.length,
                           current_token.start);
                    return false;
                }
                rx_port_pin_set = true;
            }
            // This section creates TX pin
            else if (rx_port_pin_set && !tx_port_pin_set)
            {
                if (!parsePortPin(current_token, &tx_port, &tx_pin))
                {
                    printf("Error: Unable to parse UART TX pin \"%.*s\".\r\n", current_token.length,
                           current_token.start);
                    return false;
                }
                tx_port_pin_set = true;
            }
            // If both pins are set the user has provided too many so error.
            else
            {
                printf(
                    "Error: too many port pin identifiers provided for UART initialisation.\r\n");
                return false;
            }
            break;
        }
        case TOKEN_NUMBER:
        {
            if (!baud_rate_set)
            {
                baud_rate = strtoul(current_token.start, NULL, 10);
                // This is for debugging, I will refactor it before finishing
                if ((baud_rate == 9600) || (baud_rate == 57600) || (baud_rate == 115200))
                {
                    printf("> Baud rate selected: %ul\r\n", baud_rate);
                }
                else
                {    
                    printf("Error: Baud rate must be either 9600, 57600, or 115200.\r\n");
                    return false;
                }
                baud_rate_set = true;
            }
            break;
        }
        case TOKEN_EOL:
        {
            break;
        }
        default:
        {
            printf("> Parse Error: Unrecognised token while parsing: "
                   "\"%.*s\".\r\n",
                   current_token.length, current_token.start);
            return false;
        }
        }
    }
    // If we parsed everthing properly
    if (rx_port_pin_set && tx_port_pin_set && baud_rate_set)
    {

        UARTValidPin rx_validity = getUARTInfo(rx_port, rx_pin);
        UARTValidPin tx_validity = getUARTInfo(tx_port, tx_pin);

        // Check they're both valid UART pins
        if (!rx_validity.isValid || !tx_validity.isValid)
        {
            printf("Error: one or both of the pins provided are not available as UART. Please "
                   "consult datasheet.\r\n");
            return false;
        }

        // Check both can be in the right config.
        if (!rx_validity.isRx || !tx_validity.isTx)
        {
            printf("Error: one or both of the pins provided cannot be used as TX/RX. Please "
                   "consult datasheet.\r\n");
            return false;
        }

        // Check they're part of the same UART handle

        if (!(rx_validity.handle == tx_validity.handle))
        {
            printf("Error: Pins are available as UART but not for the same UART peripheral. "
                   "Consult datasheet.\r\n");
            return false;
        }

        // Either will do at this point.
        uint32_t              handle = rx_validity.handle;
        enum rcc_periph_clken uart_clock = getUARTclock(handle);

        // These two should be the same
        enum rcc_periph_clken rx_clock = getClockFromPort(rx_port);
        enum rcc_periph_clken tx_clock = getClockFromPort(tx_port);

        // As we have to check 2 different pins, I decided it was best to get all the peripheral
        // info (clock, handle, etc) first and then check that each pin is mutatable.
        PeripheralType rx_exists = pinExists(bc, rx_port, rx_pin);
        PeripheralType tx_exists = pinExists(bc, tx_port, tx_pin);

        // Get the NVIC entry for this specific UART
        int            nvic_entry = getUARTNVICEntry(handle);
        if (nvic_entry == 0)
        {
            printf("Error: Could not find NVIC entry for that UART port.\r\n");
            return false;
        }

        // Here's where we actually build the UART peripheral
        // First option if both pins are free
        if (rx_exists == TYPE_NONE && tx_exists == TYPE_NONE)
        {
            // This means neither pin was already set up. We can set up normally.
            createUART(bc, handle, uart_clock, baud_rate, rx_port, tx_port, rx_pin, tx_pin,
                       rx_clock, tx_clock, rx_validity.af_mode, tx_validity.af_mode, nvic_entry);
            printf("> Created new UART peripheral.\r\n");
        }
        else
        {
            // If one or both pins are already initialised, this gets executed.
            if (rx_exists != TYPE_NONE && tx_exists == TYPE_NONE)
            {
                killPeripheralOrPin(bc, rx_port, rx_pin);
            }
            else if (rx_exists == TYPE_NONE && tx_exists != TYPE_NONE)
            {
                killPeripheralOrPin(bc, tx_port, tx_pin);
            }
            else
            {
                killPeripheralOrPin(bc, rx_port, rx_pin);
                killPeripheralOrPin(bc, tx_port, tx_pin);
            }

            // After all those peripherals are killed, we just build the UART as normal.
            createUART(bc, handle, uart_clock, baud_rate, rx_port, tx_port, rx_pin, tx_pin,
                       rx_clock, tx_clock, rx_validity.af_mode, tx_validity.af_mode, nvic_entry);
            printf("> Created new UART peripheral.\r\n");
        }
        return true;
    }
    else
    {
        printf("Error: could not recognise pins.\r\n");
        return false;
    }
}

/**
 * @brief UART function. Decides what to do when a UART keyword is detected.
 *
 * @param bc Board controller object
 * @param vec vector of tokens
 * @return true if uart function was successful
 * @return false if uart function was unsuccesful
 */
static bool uart(BoardController *bc, TokenVector *vec)
{
    Token next_token = getTokenVector(vec, 1);
    if (next_token.type == TOKEN_PORT_PIN)
    {
        // initialise UART
        return uartInitialise(bc, vec);
    }
    else if (next_token.type == TOKEN_GPIO_READ)
    {
        // Read UART
        char read_buffer[UART_MAX_READ];
        uint32_t read_size = readUARTPort(bc, read_buffer, (size_t)UART_MAX_READ);
        if (read_size > 0)
        {
            printf("> UART READ = \"%s\" (%ul bytes)\r\n", read_buffer, read_size);
            return true;
        }
        else {
            printf("> Error: UART buffer empty.\r\n");
            return false;
        }
    }
    else if (next_token.type == TOKEN_WRITE)
    {
        // Write UART
        next_token = getTokenVector(vec, 2);
        if (next_token.type == TOKEN_STRING)
        {
            const char *string_to_send = next_token.start;
            size_t length = (size_t)next_token.length;
            uint32_t size_written = writeUARTPort(bc, string_to_send, length);
            printf("> UART WROTE %ul BYTES.\r\n", size_written);
            return true;
        }
        else {
            printf("> Error: \"uart write\" function must be followed by string enclosed in qoutes (\").\r\n");
            return false;
        }
    }
    else
    {
        printf("> Parse Error: \"uart\" keyword must be followed by either port pin "
               "identifier, \"read\", or \"write <string>\", not \"%.*s\".",
               next_token.length, next_token.start);
        return false;
    }
}

/**
 * @brief Parses the token vector returned by interpret(). Executes any commands
 * interpreted.
 *
 * @param bc board controller object to execute commands on.
 * @param vec vector of tokens to parse
 * @return true parsing successful
 * @return false parsing unsuccessful
 */
bool parseTokensAndExecute(BoardController *bc, TokenVector *vec)
{
    // First token is always the function type identifier
    Token first_token = getTokenVector(vec, 0);
    switch (first_token.type)
    {
    case TOKEN_GPIO_INPUT:
        return inputOutput(bc, vec, OP_MAKE_INPUT);
    case TOKEN_GPIO_OUTPUT:
        return inputOutput(bc, vec, OP_MAKE_OUTPUT);
    case TOKEN_GPIO_SET:
        return setResetToggleRead(bc, vec, OP_SET);
    case TOKEN_GPIO_RESET:
        return setResetToggleRead(bc, vec, OP_RESET);
    case TOKEN_GPIO_TOGGLE:
        return setResetToggleRead(bc, vec, OP_TOGGLE);
    case TOKEN_GPIO_READ:
        return setResetToggleRead(bc, vec, OP_READ);
    case TOKEN_ADC:
        return adc(bc, vec);
    case TOKEN_UART:
        return uart(bc, vec);
    default:
    {
        printf("> Parse Error: Invalid line logic. Token \"%.*s\" is not a "
               "valid line start.\r\n",
               vec->tokens[0].length, vec->tokens[0].start);
        return false;
    }
    }
}