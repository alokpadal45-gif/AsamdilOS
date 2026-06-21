; -----------------------------------------------------
; print_pm.asm
; Direct VGA text-mode printing while in 32-bit
; protected mode (used only for the boot transition
; message - the C kernel has its own screen driver).
; -----------------------------------------------------

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY

.next_char:
    mov al, [ebx]
    mov ah, WHITE_ON_BLACK

    cmp al, 0
    je .done

    mov [edx], ax
    add ebx, 1
    add edx, 2
    jmp .next_char

.done:
    popa
    ret
