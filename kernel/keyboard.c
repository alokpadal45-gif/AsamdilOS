/* =======================================================
 * keyboard.c
 * Alok Padal - Bootloader & Kernel
 *
 * Polling PS/2 keyboard driver (US layout, scancode set 1).
 * The shell calls keyboard_getchar()/keyboard_read_line()
 * whenever it needs input; this function busy-waits on the
 * keyboard controller's status port until a key is ready,
 * then translates the scancode to ASCII.
 *
 * A polling design (rather than IRQ-driven) keeps the
 * kernel simple and avoids needing a full IDT/PIC setup,
 * while still giving a fully responsive shell in QEMU and
 * VirtualBox.
 * ======================================================= */

#include "keyboard.h"
#include "screen.h"
#include "io.h"
#include "types.h"

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

static int shift_pressed = 0;

/* Lower-case / unshifted scancode -> ASCII map (set 1) */
static const char scancode_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ',
    /* remaining entries default to 0 */
};

/* Shifted scancode -> ASCII map */
static const char scancode_ascii_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,  'A','S','D','F','G','H','J','K','L',':','"','~',
    0,  '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0, ' ',
};

#define SCAN_LSHIFT       0x2A
#define SCAN_RSHIFT       0x36
#define SCAN_LSHIFT_REL   0xAA
#define SCAN_RSHIFT_REL   0xB6

char keyboard_getchar(void) {
    while (1) {
        /* bit 0 of the status port is set when output buffer is full */
        if (inb(KBD_STATUS_PORT) & 0x01) {
            u8 scancode = inb(KBD_DATA_PORT);

            if (scancode == SCAN_LSHIFT || scancode == SCAN_RSHIFT) {
                shift_pressed = 1;
                continue;
            }
            if (scancode == SCAN_LSHIFT_REL || scancode == SCAN_RSHIFT_REL) {
                shift_pressed = 0;
                continue;
            }
            if (scancode & 0x80) {
                continue; /* ignore other key releases */
            }

            char c = shift_pressed ? scancode_ascii_shift[scancode]
                                    : scancode_ascii[scancode];
            if (c != 0) {
                return c;
            }
        }
    }
}

void keyboard_read_line(char *buf, int max_len) {
    int i = 0;

    while (1) {
        char c = keyboard_getchar();

        if (c == '\n') {
            screen_newline();
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                screen_backspace();
            }
        } else if (i < max_len - 1) {
            buf[i++] = c;
            screen_putc(c, WHITE_ON_BLACK);
        }
    }

    buf[i] = '\0';
}
