#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/cm3/systick.h"
#include "libopencm3/cm3/vector.h"

#define BUILTIN_LD2_PORT (GPIOA)
#define BUILTIN_LD2_PIN (GPIO5)
#define CPU_FREQ (84000000)
#define SYSTICK_FREQ (1000)

volatile uint64_t ticks = 0;
void sys_tick_handler(void) {
    ticks++;
}

static uint64_t get_ticks(void)
{
    return ticks;
}

static void loc_rcc_setup(void)
{
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

static void loc_gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(BUILTIN_LD2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BUILTIN_LD2_PIN);
}

static void loc_systick_setup(void)
{
    systick_set_frequency(SYSTICK_FREQ, CPU_FREQ);
    systick_counter_enable();
    systick_interrupt_enable();
}

int main(void)
{
    loc_systick_setup();
    loc_rcc_setup();
    loc_gpio_setup();

    uint64_t start_time = get_ticks();

    while (1)
    {
        if (get_ticks() - start_time >= 100) {
            gpio_toggle(BUILTIN_LD2_PORT, BUILTIN_LD2_PIN);
            start_time = get_ticks();
        }
    }


    // Should never get here
    return 0;
}