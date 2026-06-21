/* =======================================================
 * screen.c
 * Alok Padal - Bootloader & Kernel
 *
 * Direct VGA text-mode (80x25) driver. Writes characters
 * straight into video memory at 0xB8000 and keeps track
 * of the hardware cursor position via I/O ports 0x3D4/0x3D5.
 * ======================================================= */

#include "screen.h"
#include "io.h"

#define VIDEO_MEMORY ((u8*)0xb8000)
#define MAX_ROWS 25
#define MAX_COLS 80

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

static int get_cursor_offset(void) {
    outb(REG_SCREEN_CTRL, 14);
    int offset = inb(REG_SCREEN_DATA) << 8;
    outb(REG_SCREEN_CTRL, 15);
    offset += inb(REG_SCREEN_DATA);
    return offset * 2; /* convert from cell to byte offset */
}

static void set_cursor_offset(int offset) {
    offset /= 2;
    outb(REG_SCREEN_CTRL, 14);
    outb(REG_SCREEN_DATA, (u8)(offset >> 8));
    outb(REG_SCREEN_CTRL, 15);
    outb(REG_SCREEN_DATA, (u8)(offset & 0xff));
}

static int scroll_if_needed(int offset) {
    if (offset < MAX_ROWS * MAX_COLS * 2) {
        return offset;
    }

    /* shift every row up by one */
    for (int i = 1; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS * 2; j++) {
            VIDEO_MEMORY[(i - 1) * MAX_COLS * 2 + j] = VIDEO_MEMORY[i * MAX_COLS * 2 + j];
        }
    }

    /* clear the last line */
    for (int j = 0; j < MAX_COLS * 2; j += 2) {
        VIDEO_MEMORY[(MAX_ROWS - 1) * MAX_COLS * 2 + j] = ' ';
        VIDEO_MEMORY[(MAX_ROWS - 1) * MAX_COLS * 2 + j + 1] = WHITE_ON_BLACK;
    }

    return (MAX_ROWS - 1) * MAX_COLS * 2;
}

void screen_clear(void) {
    for (int i = 0; i < MAX_COLS * MAX_ROWS * 2; i += 2) {
        VIDEO_MEMORY[i] = ' ';
        VIDEO_MEMORY[i + 1] = WHITE_ON_BLACK;
    }
    set_cursor_offset(0);
}

void screen_set_cursor(int row, int col) {
    set_cursor_offset((row * MAX_COLS + col) * 2);
}

void screen_putc(char c, u8 color) {
    int offset = get_cursor_offset();

    if (c == '\n') {
        int row = offset / (2 * MAX_COLS);
        offset = (row + 1) * MAX_COLS * 2;
    } else if (c == '\r') {
        int row = offset / (2 * MAX_COLS);
        offset = row * MAX_COLS * 2;
    } else if (c == '\b') {
        offset -= 2;
        if (offset < 0) offset = 0;
        VIDEO_MEMORY[offset] = ' ';
        VIDEO_MEMORY[offset + 1] = color;
    } else {
        VIDEO_MEMORY[offset] = c;
        VIDEO_MEMORY[offset + 1] = color;
        offset += 2;
    }

    offset = scroll_if_needed(offset);
    set_cursor_offset(offset);
}

void screen_print_color(const char *str, u8 color) {
    for (int i = 0; str[i] != '\0'; i++) {
        screen_putc(str[i], color);
    }
}

void screen_print(const char *str) {
    screen_print_color(str, WHITE_ON_BLACK);
}

void screen_newline(void) {
    screen_putc('\n', WHITE_ON_BLACK);
}

void screen_backspace(void) {
    screen_putc('\b', WHITE_ON_BLACK);
}

void screen_print_dec(u32 num) {
    char buf[12];
    int i = 0;

    if (num == 0) {
        screen_putc('0', WHITE_ON_BLACK);
        return;
    }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i > 0) {
        screen_putc(buf[--i], WHITE_ON_BLACK);
    }
}

void screen_print_hex(u32 num) {
    char hex_chars[] = "0123456789ABCDEF";
    char buf[9];
    buf[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        buf[i] = hex_chars[num & 0xF];
        num >>= 4;
    }

    screen_print("0x");
    screen_print(buf);
}
