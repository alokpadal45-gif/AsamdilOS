/* =======================================================
 * shell.c
 * Samrat Karki - File System & Shell
 *
 * Implements the login screen and the interactive command
 * shell. Each supported command is parsed here and
 * dispatched to the appropriate subsystem (file system,
 * process manager, memory manager, etc.)
 * ======================================================= */

#include "shell.h"
#include "screen.h"
#include "keyboard.h"
#include "string.h"
#include "fs.h"
#include "process.h"
#include "memory.h"
#include "rtc.h"
#include "io.h"

#define USERNAME "admin"
#define PASSWORD "1234"

#define LINE_BUF_SIZE 128

void login_screen(void) {
    char user[32];
    char pass[32];

    while (1) {
        screen_clear();
        screen_print_color("==================================================\n", CYAN_ON_BLACK);
        screen_print_color("               Welcome to AsamdilOS               \n", CYAN_ON_BLACK);
        screen_print_color("==================================================\n", CYAN_ON_BLACK);
        screen_print("\n");

        screen_print("Username: ");
        keyboard_read_line(user, sizeof(user));

        screen_print("Password: ");
        /* Read password without echoing characters to the screen */
        {
            int i = 0;
            while (1) {
                char c = keyboard_getchar();
                if (c == '\n') {
                    screen_newline();
                    break;
                } else if (c == '\b') {
                    if (i > 0) i--;
                } else if (i < (int)sizeof(pass) - 1) {
                    pass[i++] = c;
                    screen_putc('*', WHITE_ON_BLACK);
                }
            }
            pass[i] = '\0';
        }

        if (strcmp(user, USERNAME) == 0 && strcmp(pass, PASSWORD) == 0) {
            screen_print_color("\nLogin successful!\n", GREEN_ON_BLACK);
            return;
        }

        screen_print_color("\nInvalid username or password. Try again.\n", RED_ON_BLACK);
        screen_print("(hint: username='admin', password='1234')\n\n");

        /* small pause so the message is readable */
        for (volatile int i = 0; i < 3000000; i++) { }
    }
}

/* ---- command implementations ---- */

static void cmd_help(void) {
    screen_print_color("Available commands:\n", CYAN_ON_BLACK);
    screen_print("  help              - show this help message\n");
    screen_print("  about             - information about AsamdilOS\n");
    screen_print("  date              - show current date and time\n");
    screen_print("  calc <a><op><b>   - simple calculator, e.g. calc 5+5\n");
    screen_print("  clear             - clear the screen\n");
    screen_print("  ls                - list files and directories\n");
    screen_print("  mkdir <name>      - create a directory\n");
    screen_print("  create <name>     - create an empty file\n");
    screen_print("  write <name> <t>  - write text t into a file\n");
    screen_print("  read <name>       - print a file's contents\n");
    screen_print("  delete <name>     - delete a file or directory\n");
    screen_print("  ps                - list running processes\n");
    screen_print("  run <name>        - create a process and run the scheduler\n");
    screen_print("  kill <pid>        - terminate a process\n");
    screen_print("  meminfo           - show memory usage information\n");
    screen_print("  sysinfo           - show system information\n");
    screen_print("  threads           - multithreading simulation demo\n");
    screen_print("  threads          - run multithreading scheduler demo\n");
    screen_print("  thread <p> <n>   - create thread for process PID p\n");
    screen_print("  shutdown          - power off the system\n");
}

static void cmd_about(void) {
    screen_print_color("=== About AsamdilOS ===\n", CYAN_ON_BLACK);
    screen_print("AsamdilOS is an educational 32-bit operating system\n");
    screen_print("built from scratch with a custom bootloader, a C\n");
    screen_print("kernel, a command shell, an in-memory file system,\n");
    screen_print("and simulated memory paging and process scheduling.\n");
    screen_print("\nDeveloped by a 3-person student team:\n");
    screen_print("  Alok Padal - Bootloader & Kernel\n");
    screen_print("  Dilraj Bista - Memory & Process Management\n");
    screen_print("  Samrat Karki - File System & Shell\n");
}

static void cmd_sysinfo(void) {
    screen_print_color("=== System Information ===\n", CYAN_ON_BLACK);
    screen_print("OS Name      : AsamdilOS\n");
    screen_print("Version      : 1.0\n");
    screen_print("Architecture : x86 (32-bit protected mode)\n");
    screen_print("Build target : QEMU / VirtualBox (raw disk image)\n");
    mem_print_info();
}

/* Very small calculator: parses "<int><op><int>" where op
 * is one of + - * / (no spaces, single operator). */
static void cmd_calc(char *expr) {
    if (expr == NULL || expr[0] == '\0') {
        screen_print_color("Usage: calc <number><op><number>  e.g. calc 5+5\n", RED_ON_BLACK);
        return;
    }

    char op = 0;
    int op_index = -1;

    for (u32 i = 0; expr[i] != '\0'; i++) {
        if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
            /* allow a leading '-' for negative first operand */
            if (expr[i] == '-' && i == 0) continue;
            op = expr[i];
            op_index = (int)i;
            break;
        }
    }

    if (op == 0) {
        screen_print_color("Error: no operator found (use + - * /)\n", RED_ON_BLACK);
        return;
    }

    char left[32];
    char right[32];

    memcpy(left, expr, op_index);
    left[op_index] = '\0';
    strcpy(right, expr + op_index + 1);

    int a = str_to_int(left);
    int b = str_to_int(right);
    int result = 0;
    int error = 0;

    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) {
                error = 1;
            } else {
                result = a / b;
            }
            break;
    }

    if (error) {
        screen_print_color("Error: division by zero\n", RED_ON_BLACK);
        return;
    }

    screen_print(left);
    screen_putc(op, WHITE_ON_BLACK);
    screen_print(right);
    screen_print(" = ");
    screen_print_dec((u32)result);
    screen_print("\n");
}

static void cmd_thread_demo(void) {
    /* Simulates 3 threads sharing CPU time in a Round Robin style.
     * Each thread has a name and a total "work" amount. We interleave
     * their execution in small steps to show concurrent-looking output,
     * which is exactly what the OS scheduler does between real threads. */

    #define NUM_THREADS 3
    #define THREAD_WORK 6
    #define THREAD_SLICE 2

    const char *tnames[NUM_THREADS] = {"Thread-A", "Thread-B", "Thread-C"};
    int remaining[NUM_THREADS] = {THREAD_WORK, THREAD_WORK, THREAD_WORK};
    int tid_pages[NUM_THREADS];

    screen_print_color("=== Multithreading Simulation ===\n", CYAN_ON_BLACK);
    screen_print("3 threads sharing CPU with Round Robin (quantum=");
    screen_print_dec(THREAD_SLICE);
    screen_print(")\n\n");

    /* allocate a page per thread to show memory usage */
    for (int i = 0; i < NUM_THREADS; i++) {
        tid_pages[i] = palloc();
        screen_print_color("[INIT] ", YELLOW_ON_BLACK);
        screen_print(tnames[i]);
        screen_print(" created, page frame ");
        screen_print_dec(tid_pages[i]);
        screen_print(" allocated\n");
    }
    screen_print("\n");

    int time = 0;
    int done;
    do {
        done = 1;
        for (int i = 0; i < NUM_THREADS; i++) {
            if (remaining[i] <= 0) continue;
            done = 0;

            int slice = (remaining[i] < THREAD_SLICE) ? remaining[i] : THREAD_SLICE;

            screen_print_color("[CPU ] ", GREEN_ON_BLACK);
            screen_print("t=");
            screen_print_dec(time);
            screen_print(": ");
            screen_print(tnames[i]);
            screen_print(" executes for ");
            screen_print_dec(slice);
            screen_print(" unit(s)  [remaining=");
            screen_print_dec(remaining[i] - slice);
            screen_print("]\n");

            time += slice;
            remaining[i] -= slice;

            if (remaining[i] == 0) {
                screen_print_color("[DONE] ", CYAN_ON_BLACK);
                screen_print(tnames[i]);
                screen_print(" finished at t=");
                screen_print_dec(time);
                screen_print("\n");
                pfree(tid_pages[i]);
            }
        }
    } while (!done);

    screen_print("\nAll threads completed. Total time = ");
    screen_print_dec(time);
    screen_print("\n");
    screen_print_color("\nNote: Real multithreading uses a hardware timer\n", YELLOW_ON_BLACK);
    screen_print_color("(PIT) to preemptively switch threads. This demo\n", YELLOW_ON_BLACK);
    screen_print_color("shows the same scheduling logic used by real OSes.\n", YELLOW_ON_BLACK);
}

static void cmd_shutdown(void) {
    screen_print_color("System is shutting down...\n", YELLOW_ON_BLACK);
    screen_print("It is now safe to close this window.\n");

    asm volatile ("cli");
    while (1) {
        asm volatile ("hlt");
    }
}

/* ---- main shell loop ---- */

void shell_run(void) {
    char line[LINE_BUF_SIZE];

    screen_print("\n");
    cmd_help();
    screen_print("\n");

    while (1) {
        screen_print_color("AsamdilOS> ", GREEN_ON_BLACK);
        keyboard_read_line(line, LINE_BUF_SIZE);

        char *args = split_first_word(line);

        if (line[0] == '\0') {
            continue;
        } else if (strcmp(line, "help") == 0) {
            cmd_help();
        } else if (strcmp(line, "about") == 0) {
            cmd_about();
        } else if (strcmp(line, "date") == 0) {
            rtc_print_date();
        } else if (strcmp(line, "clear") == 0) {
            screen_clear();
        } else if (strcmp(line, "calc") == 0) {
            cmd_calc(args);
        } else if (strcmp(line, "ls") == 0) {
            fs_list();
        } else if (strcmp(line, "mkdir") == 0) {
            if (args) fs_mkdir(args);
            else screen_print_color("Usage: mkdir <name>\n", RED_ON_BLACK);
        } else if (strcmp(line, "create") == 0) {
            if (args) fs_create(args);
            else screen_print_color("Usage: create <name>\n", RED_ON_BLACK);
        } else if (strcmp(line, "delete") == 0 || strcmp(line, "rm") == 0) {
            if (args) fs_delete(args);
            else screen_print_color("Usage: delete <name>\n", RED_ON_BLACK);
        } else if (strcmp(line, "read") == 0) {
            if (args) fs_read(args);
            else screen_print_color("Usage: read <name>\n", RED_ON_BLACK);
        } else if (strcmp(line, "write") == 0) {
            if (args) {
                char *content = split_first_word(args);
                if (content) fs_write(args, content);
                else fs_write(args, "");
            } else {
                screen_print_color("Usage: write <name> <text>\n", RED_ON_BLACK);
            }
        } else if (strcmp(line, "ps") == 0) {
            process_list();
        } else if (strcmp(line, "run") == 0) {
            const char *pname = args ? args : "process";
            process_create(pname, 6); /* default burst time of 6 units */
            schedule_fcfs();
            schedule_rr(2);
        } else if (strcmp(line, "kill") == 0) {
            if (args) process_kill(str_to_int(args));
            else screen_print_color("Usage: kill <pid>\n", RED_ON_BLACK);
        } else if (strcmp(line, "meminfo") == 0) {
            mem_print_info();
        } else if (strcmp(line, "sysinfo") == 0) {
            cmd_sysinfo();
        } else if (strcmp(line, "threads") == 0) {
            cmd_thread_demo();
        } else if (strcmp(line, "threads") == 0) {
            thread_list(-1);
            thread_schedule();
        } else if (strcmp(line, "thread") == 0) {
            if (args) {
                char *tname = split_first_word(args);
                int pid = str_to_int(args);
                if (tname) thread_create(pid, tname, 4);
                else screen_print_color("Usage: thread <pid> <name>\n", RED_ON_BLACK);
            } else {
                screen_print_color("Usage: thread <pid> <name>\n", RED_ON_BLACK);
            }
        } else if (strcmp(line, "shutdown") == 0) {
            cmd_shutdown();
        } else if (strcmp(line, "login") == 0) {
            login_screen();
            screen_clear();
            screen_print_color("Login successful. Type 'help' for commands.\n\n", GREEN_ON_BLACK);
        } else {
            screen_print_color("Unknown command: ", RED_ON_BLACK);
            screen_print(line);
            screen_print("\n");
            screen_print("Type 'help' to see the list of available commands.\n");
        }
    }
}
