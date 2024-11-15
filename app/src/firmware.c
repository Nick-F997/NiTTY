#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/cm3/scb.h" // For vector table offset register

#include "core/system.h"
#include "core/uart.h"
#include "sys_timer.h"

#include <stdio.h>

#include "board-control.h"

#define BOOTLOADER_SIZE     (0x8000U)

#define BUILTIN_LD2_PORT    (GPIOA)
#define BUILTIN_LD2_PIN     (GPIO5)




static void loc_vector_setup(void) {
    SCB_VTOR = BOOTLOADER_SIZE; // Offset main Vector Table by size of bootloader so it knows where to look.
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
        actionDigitalPin(board, BUILTIN_LD2_PORT, BUILTIN_LD2_PIN, GPIO_SET);
        printf("Set pin...\r\n");
        coreSystemDelay(500);
        actionDigitalPin(board, BUILTIN_LD2_PORT, BUILTIN_LD2_PIN, GPIO_CLEAR);
        printf("Cleared pin...\r\n");
        coreSystemDelay(500);
    }

    deinitBoard(board);
    // Should never get here
    return 0;
}