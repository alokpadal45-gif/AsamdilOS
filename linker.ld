/* =======================================================
 * linker.ld
 * Links the kernel as a flat binary starting at 0x1000,
 * matching KERNEL_OFFSET in boot/boot.asm (the address
 * where the bootloader loads the kernel from disk).
 * ======================================================= */

ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text :
    {
        *(.text*)
    }

    .data :
    {
        *(.data*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .bss :
    {
        *(.bss*)
        *(COMMON)
    }

    kernel_end = .;
}
