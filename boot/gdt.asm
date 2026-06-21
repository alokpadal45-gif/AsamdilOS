; -----------------------------------------------------
; gdt.asm
; Minimal flat GDT: one code segment and one data
; segment, both covering the full 4GB address space.
; -----------------------------------------------------

gdt_start:

gdt_null:               ; mandatory null descriptor
    dd 0x0
    dd 0x0

gdt_code:               ; code segment descriptor
    ; base = 0x00000000, limit = 0xfffff
    dw 0xffff           ; limit (bits 0-15)
    dw 0x0              ; base  (bits 0-15)
    db 0x0              ; base  (bits 16-23)
    db 10011010b        ; flags: present, ring0, code, executable, readable
    db 11001111b        ; flags: granularity 4K, 32-bit, limit (bits 16-19)
    db 0x0              ; base  (bits 24-31)

gdt_data:               ; data segment descriptor
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b        ; present, ring0, data, writable
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; size of GDT - 1
    dd gdt_start                 ; start address of GDT

; convenient constants for segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
