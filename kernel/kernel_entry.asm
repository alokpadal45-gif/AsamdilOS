; =======================================================
; kernel_entry.asm
; Alok Padal - Bootloader & Kernel
;
; This is the very first code executed once the
; bootloader jumps to the kernel at 0x1000. It simply
; hands control to the C function kernel_main().
; =======================================================

[bits 32]
[extern kernel_main]

global _start
_start:
    call kernel_main
    jmp $          ; should never return, but halt forever just in case
