#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"


#define BUILTIN_LD2_PORT (GPIOA)
#define BUILTIN_LD2_PIN (GPIO5)

static void loc_rcc_setup(void)
{
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

static void loc_gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(BUILTIN_LD2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BUILTIN_LD2_PIN);
}

static void loc_delay(uint32_t cycles)
{
    for (uint32_t i = 0; i < cycles; i++)
    {
        __asm__("nop");
    }
}

int main(void)
{
    loc_rcc_setup();
    loc_gpio_setup();

    while (1)
    {
        for (uint32_t i = 0; i < 84000000; i+= 10000000)
        {
            gpio_toggle(BUILTIN_LD2_PORT, BUILTIN_LD2_PIN);
            loc_delay((uint32_t)(i / 8));
        }
    }


    // Should never get here
    return 0;
}