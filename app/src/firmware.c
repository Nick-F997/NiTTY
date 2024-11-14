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

#define UART_PORT           (GPIOA)
#define UART_TX_PIN         (GPIO2)
#define UART_RX_PIN         (GPIO3)


static void loc_vector_setup(void) {
    SCB_VTOR = BOOTLOADER_SIZE; // Offset main Vector Table by size of bootloader so it knows where to look.
}

static void loc_gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    // GPIO setup for uart. Clock for peripheral is set elsewhere
    gpio_mode_setup(UART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, UART_TX_PIN | UART_RX_PIN /* boolean or in order to set both pins*/);
    gpio_set_af(UART_PORT, GPIO_AF7, UART_TX_PIN | UART_RX_PIN); // See datasheet for function.
}

int main(void)
{
    loc_vector_setup();
    coreSystemSetup();
    loc_gpio_setup();
    
    coreUartSetup(115200);

    BoardController *board = initBoard();
    createDigitalPin(board, BUILTIN_LD2_PORT, BUILTIN_LD2_PIN, RCC_GPIOA, false, GPIO_PUPD_NONE);
    
    while (1)
    {
        action_DigitalOutputPin(board, BUILTIN_LD2_PORT, BUILTIN_LD2_PIN, GPIO_SET);
        printf("Set pin...\r\n");
        coreSystemDelay(500);
        action_DigitalOutputPin(board, BUILTIN_LD2_PORT, BUILTIN_LD2_PIN, GPIO_CLEAR);
        printf("Cleared pin...\r\n");
        coreSystemDelay(500);
    }

    deinitBoard(board);
    // Should never get here
    return 0;
}