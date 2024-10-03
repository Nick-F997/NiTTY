#include "common-includes.h"
#include "libopencm3/stm32/memorymap.h"

#define BOOTLOADER_SIZE (0x8000U)
#define MAIN_APP_START_ADDR (FLASH_BASE + BOOTLOADER_SIZE)

static void jumpToMain(void) {
    typedef void (*void_fn)(void); // Function pointer for main() in main_app.
    
    uint32_t *reset_vector_entry = (uint32_t *)(MAIN_APP_START_ADDR + 4U); // start of main app + 4 bytes
    uint32_t *reset_vector = (uint32_t *)(*reset_vector_entry); // Find address at rest_vec entry, cast to pointer so we can treat it as mem.

    void_fn jump_fn = (void_fn)reset_vector; // Cast that value as a function

    jump_fn(); // Call that function
}

int main(void) {
    jumpToMain();
    // We will never return as we will jump to a different point
    return 0;
}