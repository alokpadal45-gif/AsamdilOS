#ifndef IO_H
#define IO_H

#include "types.h"

/* Write a byte to an I/O port */
static inline void outb(u16 port, u8 val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Read a byte from an I/O port */
static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Small delay used after PIC/CMOS port writes */
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif
