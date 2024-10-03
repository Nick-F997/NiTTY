/**
 * @file bootloader.c
 * @author Nicholas Fairburn (nicholas2.fairburn@live.uwe.ac.uk)
 * @brief Main functionality of the bootloader. Jumps to the main function in the firmware.
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "common-includes.h"
#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/cm3/vector.h"

// Bootloader is 32kB
#define BOOTLOADER_SIZE (0x8000U)
#define MAIN_APP_START_ADDR (FLASH_BASE + BOOTLOADER_SIZE)

/**
 * @brief Jumps to the main() function in app/src/firmware.c. Be careful when casting memory locations to structs. 
 * Simplified to use libopencm3 structs to call the reset vector in the firmware.
 * 
 */
static void jump_to_main(void) {
    vector_table_t *main_vector_table = (vector_table_t*)(MAIN_APP_START_ADDR); // Uses a type from CM3. Defined in vector.h
    main_vector_table->reset(); // Call the main function.
}

int main(void) {
    jump_to_main();
    // We will never return as we will jump to a different point
    return 0;
}