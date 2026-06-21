; -----------------------------------------------------
; print.asm
; Simple 16-bit real mode string printing using BIOS
; interrupt 0x10 (teletype output)
; -----------------------------------------------------

print_string:
    pusha
    mov ah, 0x0e        ; BIOS teletype function

.next_char:
    mov al, [si]
    cmp al, 0
    je .done
    int 0x10
    inc si
    jmp .next_char

.done:
    popa
    ret
