#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/cm3/scb.h" // For vector table offset register

#include "core/system.h"
#include "core/uart.h"
#include "sys_timer.h"

#include <stdio.h>

#include "board-control.h"
#include "interpreter.h"

#define BOOTLOADER_SIZE     (0x8000U)

#define BUILTIN_LD2_PORT    (GPIOA)
#define BUILTIN_LD2_PIN     (GPIO5)

static void loc_vector_setup(void) {
    SCB_VTOR = BOOTLOADER_SIZE; // Offset main Vector Table by size of bootloader so it knows where to look.
}

static void clearLine(char *line, size_t len)
{
    for (size_t i = 0; i < len; i++)
        line[i] = 0;
}

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
            printf("\n> %s\n", line);
            line[count > 0 ? count - 1 : 0] = '\0';
            if (!interpret(bc, line, count))
            {
                printf("Could not interpret line: \"%s\".\r\n", line);
            }
            clearLine(line, count);
            count = 0;
        }
    }
}

int main(void)
{
    // Setup local bootloader offset
    loc_vector_setup();
    // Set up systick and timer
    coreSystemSetup();

    // Setup reserved UART port 2.    
    coreUartSetup(115200);
    
    BoardController *board = initBoard();
    createDigitalPin(board, BUILTIN_LD2_PORT, BUILTIN_LD2_PIN, RCC_GPIOA, TYPE_GPIO_OUTPUT, GPIO_PUPD_NONE);
    
    while (1)
    {
        repl(board);
    }

    deinitBoard(board);
    // Should never get here
    return 0;
}