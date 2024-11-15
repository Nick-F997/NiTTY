/**
 * @file board-control.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Source code for all functions relating to BoardController and general board control.
 * @version 0.1
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "board-control.h"

/**
 * @brief Initialise the board object. To be called at startup.
 * 
 * @return BoardController* main board controller.
 */
BoardController* initBoard(void)
{
    BoardController *bc = (BoardController *)malloc(sizeof(BoardController));
    
    // vect
    bc->clocks_size = 4;
    bc->peripherals_size = 4;
    bc->peripherals_count = 0;
    bc->clocks_count = 0;

    bc->clocks = (ClockController *)malloc(sizeof(ClockController) * bc->clocks_size);
    bc->peripherals = (PeripheralController *)malloc(sizeof(PeripheralController) * bc->peripherals_size);

    return bc;
}

/**
 * @brief deinitialises a board control object. Also disables all peripherals.
 * 
 * @param bc board control object. 
 */
void deinitBoard(BoardController *bc)
{
    for (size_t clock = 0; clock < bc->clocks_count; clock++)
    {
        disableClock(&bc->clocks[clock]);
    }
    bc->clocks_count = 0;
    free(bc->clocks);

    for (size_t peripheral = 0; peripheral < bc->peripherals_count; peripheral++)
    {
        // Disable peripheral
        bc->peripherals[peripheral].disablePeripheral(&bc->peripherals[peripheral]);
    }
    bc->peripherals_count = 0;
    free(bc->peripherals);
    free(bc);
}

/**
 * @brief static function to grow the size of the .clocks member and add a new element.
 * 
 * @param bc board control object
 * @param clock clock to be added
 */
static void growClocks(BoardController *bc, enum rcc_periph_clken clock)
{
    if (bc->clocks_count == bc->clocks_size)
    {
        size_t oldSize = bc->clocks_size;
        bc->clocks_size = GROW_CAPACITY(oldSize);
        bc->clocks = GROW_ARRAY(ClockController, bc->clocks, oldSize, bc->clocks_size);
    }
    bc->clocks[bc->clocks_count++] = create_clock(clock); 
}

/**
 * @brief Grows and adds new element to .peripherals member
 * 
 * @param bc board control structure
 * @param periph peripheral to be added
 */
static void growPeripherals(BoardController *bc, PeripheralController periph)
{
    if (bc->peripherals_count == bc->peripherals_size)
    {
        size_t oldSize = bc->peripherals_size;
        bc->peripherals_size = GROW_CAPACITY(oldSize);
        bc->peripherals = GROW_ARRAY(PeripheralController, bc->peripherals, oldSize, bc->peripherals_size);
    }
    bc->peripherals[bc->peripherals_count++] = periph;
}

/**
 * @brief returns whether a clock object already exists for given clock
 * 
 * @param bc board control object
 * @param clock clock to check
 * @return true clock already created
 * @return false clock not created
 */
static bool clockExists(BoardController *bc, enum rcc_periph_clken clock)
{
    for (size_t clock_c = 0; clock_c < bc->clocks_count; clock_c++)
    {
        if (bc->clocks[clock_c].clock == clock)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Creates a new digital GPIO pin. 
 * 
 * @param bc board control object
 * @param port GPIO port
 * @param pin GPIO pin
 * @param clock RCC clock 
 * @param input_output is it input or output True = input, false = output 
 * @param pupd does it have a pullup/pulldown resistor
 */
void createDigitalPin(BoardController *bc, uint32_t port, uint32_t pin,
                enum rcc_periph_clken clock, PeripheralType input_output, uint8_t pupd)
{
    // if clock is already made and enabled do nothing.
    bool clock_exists = clockExists(bc, clock);

    // if clock doesn't exist create it and enable it.
    if (!clock_exists)
    {
        growClocks(bc, clock);
        enableClock(&bc->clocks[bc->clocks_count - 1]);
    }

    // Create peripheral and enable it. 
    PeripheralController pc = createStandardGPIO(port, pin, clock, input_output, pupd);
    growPeripherals(bc, pc);
    bc->peripherals[bc->peripherals_count - 1].enablePeripheral(&bc->peripherals[bc->peripherals_count - 1]);
}

/**
 * @brief Change the function of a digital pin
 * 
 * @param bc board control
 * @param port port to change
 * @param pin pin to change
 * @param new_type new type of pin, either input or output
 * @param new_pupd new pullup/pulldown resistor
 */
void mutateDigitalPin(BoardController *bc, uint32_t port, uint32_t pin, PeripheralType new_type, uint8_t new_pupd)
{
    for (size_t periph = 0; periph < bc->peripherals_count; periph++)
    {
        PeripheralController *current_periph = &bc->peripherals[periph];
        if (current_periph->type == TYPE_GPIO_INPUT || current_periph->type == TYPE_GPIO_OUTPUT)
        {
            if (current_periph->peripheral.gpio.port == port && current_periph->peripheral.gpio.pin == pin)
            {
                if (current_periph->type == new_type)
                {
                    return;
                }

                current_periph->disablePeripheral(current_periph);
                *current_periph = createStandardGPIO(port, pin, current_periph->peripheral.gpio.clock, new_type, new_pupd);
                current_periph->enablePeripheral(current_periph);
                return;
            }
        }
    }
}

/**
 * @brief conducts an action on digital gpio pin
 * 
 * @param bc Main board structure
 * @param port port to action
 * @param pin pin to action
 * @param action action to be carried out.
 */
void actionDigitalPin(BoardController *bc, uint32_t port, uint32_t pin, GPIOAction action)
{
    for (size_t periph = 0; periph < bc->peripherals_count; periph++)
    {
        PeripheralController *current_periph = &bc->peripherals[periph];
        if (current_periph->peripheral.gpio.port == port && current_periph->peripheral.gpio.pin == pin)
        {
            switch (current_periph->type)
            {
                case TYPE_GPIO_INPUT:
                {
                    if (action == GPIO_READ)
                    {
                        gpio_get(port, pin);
                    }
                    return;            
                }
                case TYPE_GPIO_OUTPUT:
                {
                    switch (action)
                    {
                        case GPIO_SET:
                        {
                            gpio_set(port, pin);
                            break;
                        }
                        case GPIO_CLEAR:
                        {
                            gpio_clear(port, pin);
                            break;
                        }
                        case GPIO_TOGGLE:
                        {
                            gpio_toggle(port, pin);
                            break;
                        }
                    }
                    return;
                }
            }
        }    
    }
}
        