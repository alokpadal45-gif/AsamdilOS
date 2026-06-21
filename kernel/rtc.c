/* =======================================================
 * rtc.c
 * Samrat Karki - File System & Shell
 *
 * Reads the current date and time from the motherboard's
 * CMOS Real Time Clock chip (ports 0x70/0x71) for the
 * "date" shell command. Values are stored in BCD format
 * by the hardware, so they are converted to binary before
 * being printed.
 * ======================================================= */

#include "rtc.h"
#include "screen.h"
#include "io.h"
#include "types.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static u8 cmos_read(u8 reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

static int bcd_to_bin(u8 val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

static void print2(int n) {
    if (n < 10) screen_print("0");
    screen_print_dec((u32)n);
}

void rtc_print_date(void) {
    int second = bcd_to_bin(cmos_read(0x00));
    int minute = bcd_to_bin(cmos_read(0x02));
    int hour   = bcd_to_bin(cmos_read(0x04));
    int day    = bcd_to_bin(cmos_read(0x07));
    int month  = bcd_to_bin(cmos_read(0x08));
    int year   = bcd_to_bin(cmos_read(0x09));

    screen_print("Date: ");
    print2(day);
    screen_print("/");
    print2(month);
    screen_print("/20");
    print2(year);

    screen_print("   Time: ");
    print2(hour);
    screen_print(":");
    print2(minute);
    screen_print(":");
    print2(second);
    screen_print(" (UTC)\n");
}
