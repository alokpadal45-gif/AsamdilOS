#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

/* VGA text mode colour attributes */
#define WHITE_ON_BLACK   0x0f
#define GREEN_ON_BLACK   0x0a
#define RED_ON_BLACK     0x0c
#define CYAN_ON_BLACK    0x0b
#define YELLOW_ON_BLACK  0x0e

void screen_clear(void);
void screen_putc(char c, u8 color);
void screen_print(const char *str);
void screen_print_color(const char *str, u8 color);
void screen_print_dec(u32 num);
void screen_print_hex(u32 num);
void screen_set_cursor(int row, int col);
void screen_backspace(void);
void screen_newline(void);

#endif
