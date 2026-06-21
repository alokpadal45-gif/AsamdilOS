/* =======================================================
 * kernel.c
 * Alok Padal - Bootloader & Kernel
 *
 * kernel_main() is called by kernel_entry.asm once the
 * bootloader has switched the CPU into 32-bit protected
 * mode. It initializes every subsystem (screen, interrupts,
 * keyboard, memory, processes, file system) and then hands
 * control to the login screen and shell.
 * ======================================================= */

#include "screen.h"
#include "memory.h"
#include "process.h"
#include "fs.h"
#include "shell.h"

void kernel_main(void) {
    screen_clear();

    screen_print_color("AsamdilOS booting...\n", GREEN_ON_BLACK);
    screen_print("Initializing memory manager...\n");
    mem_init();

    screen_print("Initializing process table...\n");
    process_init();

    /* spawn built-in system processes - visible in ps like a real OS */
    process_spawn_system("init",      2, 2);  /* PID 1 - parent of all */
    process_spawn_system("idle",      1, 1);  /* PID 2 - runs when CPU free */
    process_spawn_system("klogd",     2, 1);  /* PID 3 - kernel log daemon */
    process_spawn_system("scheduler", 3, 2);  /* PID 4 - scheduling daemon */
    process_spawn_system("memwatch",  2, 1);  /* PID 5 - memory monitor */

    screen_print("Initializing file system...\n");
    fs_init();

    screen_print("Kernel initialization complete.\n\n");

    /* small pause so the boot messages are readable */
    for (volatile int i = 0; i < 5000000; i++) { }

    login_screen();

    screen_clear();
    screen_print_color("Login successful. Type 'help' for a list of commands.\n\n", GREEN_ON_BLACK);

    shell_run();

    /* shell_run() never returns, but just in case: */
    while (1) {
        asm volatile ("hlt");
    }
}
