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
#include "libopencm3/stm32/f4/gpio.h"
#include "peripheral-controller.h"
#include <stdint.h>
#include <stdio.h>

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
 * @brief Returns the channel from the port/pin identifier. Developed based on stm32-nucleoF411RE datasheets.
 * 
 * @param port port to identify
 * @param pin pin to identify
 * @return int The ADC channel
 */
static int getADCFromPortPin(uint32_t port, uint32_t pin)
{
    switch (port)
    {
        case GPIOA:
        {
            switch (pin)
            {
            case GPIO0: return ADC_CHANNEL0;
            case GPIO1: return ADC_CHANNEL1;
            case GPIO4: return ADC_CHANNEL4;
            case GPIO5: return ADC_CHANNEL5;
            case GPIO6: return ADC_CHANNEL6;
            case GPIO7: return ADC_CHANNEL7;
            default: break;
            }
            break;
        }
        case GPIOB:
        {
            switch (pin)
            {
            case GPIO0: return ADC_CHANNEL8;
            case GPIO1: return ADC_CHANNEL9;
            default: break;
            }
            break;
        }
        case GPIOC:
        {
            switch (pin)
            {
            case GPIO0: return ADC_CHANNEL10;
            case GPIO1: return ADC_CHANNEL11;
            case GPIO2: return ADC_CHANNEL12;
            case GPIO3: return ADC_CHANNEL13;
            case GPIO4: return ADC_CHANNEL14;
            case GPIO5: return ADC_CHANNEL15;
            default: break;
            }
            break;
        }
        default: break;
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
                enum rcc_periph_clken clock = getClockFromPort(port);
                mutateADCToDigital(bc, port, pin, clock, type_input_output, pupd);
                printf("> Modified ADC to GPIO pin.\r\n");
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
                    printf("> Reading ADC... (placeholder).\r\n");
                    break;
                }
                default:
                {
                    printf("> Parse Error: this operation is unavailable for this pin configuration (ADC).\r\n");
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
        printf("ADC Token - placeholder.\r\n");
        return true;
    default:
    {
        printf("> Parse Error: Invalid line logic. Token \"%.*s\" is not a "
               "valid line start.\r\n",
               vec->tokens[0].length, vec->tokens[0].start);
        return false;
    }
    }
}