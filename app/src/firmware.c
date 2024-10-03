#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/cm3/scb.h" // For vector table offset register

#include "core/system.h"
#include "sys_timer.h"

#define BOOTLOADER_SIZE (0x8000U)

#define BUILTIN_LD2_PORT (GPIOA)
#define BUILTIN_LD2_PIN (GPIO5)

static void loc_vector_setup(void) {
    SCB_VTOR = BOOTLOADER_SIZE; // Offset main Vector Table by size of bootloader so it knows where to look.
}

static void loc_gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(BUILTIN_LD2_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BUILTIN_LD2_PIN);
    gpio_set_af(BUILTIN_LD2_PORT, GPIO_AF1, BUILTIN_LD2_PIN);
}

int main(void)
{
    loc_vector_setup();
    coreSystemSetup();
    loc_gpio_setup();
    coreTimerSetup();

    uint64_t start_time = coreGetTicks();
    float duty_cycle = 0.0f;
    float incrementer = 1.0f;

    corePWMSetDutyCycle(duty_cycle);

    while (1)
    {
        if (coreGetTicks() - start_time >= 1)
        {
            duty_cycle += incrementer;
            if (duty_cycle > 100.0f || duty_cycle <= 0.0f)
            {
                incrementer *= -1.0f;
            }

            corePWMSetDutyCycle(duty_cycle);
            start_time = coreGetTicks();
        }
    }

    // Should never get here
    return 0;
}