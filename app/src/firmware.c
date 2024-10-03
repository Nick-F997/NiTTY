#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"

#include "core/system.h"
#include "core/timer.h"

#define BUILTIN_LD2_PORT (GPIOA)
#define BUILTIN_LD2_PIN (GPIO5)

static void loc_gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(BUILTIN_LD2_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BUILTIN_LD2_PIN);
    gpio_set_af(BUILTIN_LD2_PORT, GPIO_AF1, BUILTIN_LD2_PIN);
}

int main(void)
{
    coreSystemSetup();
    loc_gpio_setup();
    coreTimerSetup();

    uint64_t start_time = coreGetTicks();
    float duty_cycle = 0.0f;
    float incrementer = 1.0f;

    corePWMSetDutyCycle(duty_cycle);

    while (1)
    {
        if (coreGetTicks() - start_time >= 10)
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