/**
 * @file firmware.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Main file for execution. All roads lead on from here!
 * @version 0.1
 * @date 2024-11-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

// libgcc includes
#include <stdio.h>

// libopencm3 includes
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/cm3/scb.h" // For vector table offset register

// local includes
#include "core/system.h"
#include "core/uart.h"
#include "sys_timer.h"
#include "board-control.h"
#include "interpreter.h"
#include "version.h"

// Size of the bootloader binary
#define BOOTLOADER_SIZE     (0x8000U)

// Built in LED ports, for testing.
#define BUILTIN_LD2_PORT    (GPIOA)
#define BUILTIN_LD2_PIN     (GPIO5)

/**
 * @brief Function that tells the system to skip the bootloader on main() execution
 * 
 */
static void loc_vector_setup(void) {
    SCB_VTOR = BOOTLOADER_SIZE; // Offset main Vector Table by size of bootloader so it knows where to look.
}

/**
 * @brief Prints the NiTTY logo. 
 * 
 */
static void print_logo(void)
{
    printf(" _____________________________\r\n");
    printf("(  _   _ _ _____ _______   __ )\r\n");
    printf("| | \\ | (_|_   _|_   _\\ \\ / / |\r\n");
    printf("| |  \\| | | | |   | |  \\ V /  |\r\n");
    printf("| | |\\  | | | |   | |   | |   |\r\n");
    printf("| |_| \\_|_| |_|   |_|   |_|   |\r\n");
    printf("(_____________________________)\r\n");
    printf("Version: %s.%s\r\n", VERSION_MJR, VERSION_MIN);
    printf("Git commit: %s\r\n", GIT_VERSION);
}

/**
 * @brief helper function to clear the repl buffer
 * 
 * @param line buffer to clear
 * @param len length of buffer
 */
static void clearLine(char *line, size_t len)
{
    for (size_t i = 0; i < len; i++)
        line[i] = 0;
}

/**
 * @brief Main function to create repl interface. After line is completed, pass to interpret.
 * 
 * @param bc board controller object.
 */
static void repl(BoardController *bc)
{
    static char line[32]; 
    static size_t count = 0;

    while (coreUartDataAvailable())
    {
        char byte = (char)coreUartReadByte();
        coreUartWriteByte(byte);
        line[count++] = byte;
        if (byte == '\r')
        {
            //printf("\n> ");
            printf("\r\n");
            line[count > 0 ? count - 1 : 0] = '\0';
            if (!interpret(bc, line, count))
            {
                printf("> Failed to execute line: \"%s\".\r\n", line);
            }
            clearLine(line, count);
            count = 0;
        }
    }
}

/**
 * @brief Main function.
 * 
 * @return int should never return this.
 */
int main(void)
{
    // Setup local bootloader offset
    loc_vector_setup();
    // Set up systick and timer
    coreSystemSetup();

    // Setup reserved UART port 2.    
    coreUartSetup(115200);
    
    print_logo();
    printf("-- Welcome to NiTTY --\r\n");
    printf("Please see documentation for any help!\r\n");
    BoardController *board = initBoard();
    //createDigitalPin(board, BUILTIN_LD2_PORT, BUILTIN_LD2_PIN, RCC_GPIOA, TYPE_GPIO_OUTPUT, GPIO_PUPD_NONE);
    
    while (1)
    {
        // Sit in the repl.
        repl(board);
    }

    deinitBoard(board);
    // Should never get here
    return 0;
}