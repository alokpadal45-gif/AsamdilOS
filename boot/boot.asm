; =======================================================
;  AsamdilOS Bootloader
;  Alok Padal - Bootloader & Kernel
;
;  Responsibilities:
;   1. Print a real-mode status message
;   2. Load the kernel from disk into memory
;   3. Switch the CPU into 32-bit protected mode
;   4. Hand control over to the C kernel
; =======================================================

[org 0x7c00]
KERNEL_OFFSET equ 0x1000      ; address where the kernel is loaded
KERNEL_START  equ KERNEL_OFFSET

    mov [BOOT_DRIVE], dl       ; BIOS stores boot drive number in dl

    mov bp, 0x9000             ; set up a small stack for real mode
    mov sp, bp

    mov si, MSG_REAL_MODE
    call print_string

    call load_kernel           ; load the kernel from disk
    call switch_to_pm           ; switch to protected mode (never returns)

    jmp $                       ; safety net, should never be reached

; ---- 16-bit helper routines ----
%include "boot/print.asm"
%include "boot/disk.asm"
%include "boot/gdt.asm"
%include "boot/switch_pm.asm"
%include "boot/print_pm.asm"

[bits 16]
load_kernel:
    mov si, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET   ; ES:BX = destination for kernel image
    mov dh, 40              ; number of sectors to read (40 * 512 = 20KB)
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

; ---- data ----
BOOT_DRIVE      db 0
MSG_REAL_MODE   db "AsamdilOS bootloader: starting in 16-bit real mode...", 13, 10, 0
MSG_LOAD_KERNEL db "AsamdilOS bootloader: loading kernel from disk...", 13, 10, 0

; ---- boot sector padding & signature ----
times 510-($-$$) db 0
dw 0xaa55
