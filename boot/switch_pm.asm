; -----------------------------------------------------
; switch_pm.asm
; Enable the A20 line, load the GDT, set the protection
; enable bit in CR0 and far-jump into 32-bit code.
; -----------------------------------------------------

[bits 16]
switch_to_pm:
    cli                     ; disable interrupts - no IDT set up yet

    ; ---- enable the A20 line (fast A20 gate) ----
    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [gdt_descriptor]  ; load our GDT

    mov eax, cr0
    or eax, 0x1            ; set the PE (protection enable) bit
    mov cr0, eax

    jmp dword CODE_SEG:BEGIN_PM   ; far jump to flush the instruction cache
                                  ; and reload CS with the 32-bit code selector

[bits 32]
BEGIN_PM:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000        ; new 32-bit stack base
    mov esp, ebp

    call KERNEL_START       ; jump to the C kernel
