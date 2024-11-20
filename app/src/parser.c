#include "parser.h"

/**
 * @brief Parses a GPIO pin from a given token. Some notes on execution (also covered in code below):
 *        Ports are calculated as some offset from the peripheral base. For example, GPIOA is 
 *        (((0x40000000U) + 0x20000) + 0x0000), where 0x0000 is the offset. The offset value is 0x400, 
 *        Which is saved as PORT_SIZE. We multiply this by the current character minus the first character (subtracter).
 *        Each pin is indexed in a port by a bit shift along. E.g. GPIO0 = (1 << 0), GPIO1 = (1 << 1), GPIO2 = (1 << 2), etc.
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
            // Ports are calculated as some offset from the peripheral base. For example, GPIOA is 
            // (((0x40000000U) + 0x20000) + 0x0000), where 0x0000 is the offset. The offset value is 0x400, 
            // Which is saved as PORT_SIZE. We multiply this by the current character minus the first character (subtracter).
            *port += PORT_SIZE * ((uint32_t)port_char - subtractor);
            parsed_port = true;
            break;
        }
        // If we hit 'E', we jump to 'a'. We then update subtractor to essentially loop back to the start, as a10 == A10.
        if (port_char == PORTE_STM32F411RE)
        {
            port_char += JUMP_TO_LOWERCASE;
            subtractor = (uint32_t)PORTa_STM32F411RE;
        }
    }

    // Increment away from the port identifier
    token.start++;
    // Parse remaining value.
    // Error checking could be done here but as mentioned it should already be correct.
    uint32_t pin_val = strtoul(token.start, NULL, 10);

    // This should never be outside of this range as it's checked in the scanner but better safe than sorry.
    if (pin_val >= 0 && pin_val <= 15)
    {
        // Calculate pin value mathematically
        // Each pin is indexed in a port by a bit shift along. E.g. GPIO0 = (1 << 0), GPIO1 = (1 << 1), GPIO2 = (1 << 2), etc.
        *pin = (1 << pin_val);
        parsed_pin = true;
    }
    return parsed_port && parsed_pin; // Return false if either parse failed.
}


static bool inputOutput(BoardController *bc, TokenVector *vec, PeripheralType input_output)
{   
    size_t vec_size = sizeTokenVector(vec);
    // Ignore end of line token.
    if (vec_size - 1 != INPUT_OUTPUT_MAX_ARGS)
    {
        printf("Invalid input format, use \"input <port pin> <pup/pdown/none>\". See documentation for more information.\r\n");
        return false;
    }

    // init empty vars, we check for null later to make sure all were assigned.
    uint8_t pupd = 0;
    uint32_t port = 0;
    uint32_t pin = 0;
    bool port_pin_set = false;
    bool pupd_set = false;

    for (size_t i = 1; i < vec_size; i++)
    {
        Token current_token = getTokenVector(vec, i);
        switch (current_token.type)
        {
            case TOKEN_EOL:
            {
                break;
            }
            case TOKEN_PORT_PIN:
            {
                if (!port_pin_set)
                {
                    if (!parsePortPin(current_token, &port, &pin))
                    {
                        printf("Parse Error: Unable to parse GPIO identifer \"%.*s\".", current_token.length, current_token.start);
                        return false;
                    }
                    port_pin_set = true;
                }
                else
                {
                    printf("Parse Error: Multiple GPIO pins provided.\r\n");
                    return false;
                }

                break;
            }
            case TOKEN_GPIO_NORESISTOR:
            case TOKEN_GPIO_PULLUP:
            case TOKEN_GPIO_PULLDOWN:
            {
                if (!pupd_set)
                {
                    pupd = (current_token.type == TOKEN_GPIO_NORESISTOR) ? GPIO_PUPD_NONE :
                           (current_token.type == TOKEN_GPIO_PULLUP) ? GPIO_PUPD_PULLUP : GPIO_PUPD_PULLDOWN;
                    pupd_set = true;
                }
                else 
                {
                    printf("Parse Error: Multiple pullup/pulldown resistor configurations provided.\r\n");
                    return false;
                }
                break;
            }
            default:
            {
                printf("Parse Error: Unrecognised token while parsing: \"%.*s\".\r\n", current_token.length, current_token.start);
                return false;
            }
        }
    }
}



    


bool parseTokens(BoardController *bc, TokenVector *vec)
{
    // First token is always the function type identifier
    Token first_token = getTokenVector(vec, 0);
    switch (first_token.type)
    {
        case TOKEN_GPIO_INPUT:
        {

        }
        case TOKEN_GPIO_OUTPUT:
        {

        }
        case TOKEN_GPIO_SET:
        {

        }
        case TOKEN_GPIO_RESET:
        {

        }
        case TOKEN_GPIO_TOGGLE:
        {

        }
        default:
        {
            printf("Invalid line logic. Token \"%.*s\" is not a valid line start.\r\n", vec->tokens[0].length, vec->tokens[0].start);
            return false;
        }
    }
}