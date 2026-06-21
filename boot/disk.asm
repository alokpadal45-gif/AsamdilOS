; -----------------------------------------------------
; disk.asm
; Load DH sectors from drive DL into ES:BX using
; BIOS interrupt 0x13 (disk read service)
; -----------------------------------------------------

disk_load:
    pusha
    push dx            ; remember number of sectors requested

    mov ah, 0x02       ; BIOS read sector function
    mov al, dh         ; number of sectors to read
    mov ch, 0x00       ; cylinder 0
    mov cl, 0x02       ; start reading from sector 2 (sector 1 = boot sector)
    mov dh, 0x00       ; head 0

    int 0x13           ; BIOS disk interrupt
    jc disk_error      ; carry flag set -> error

    pop dx
    cmp al, dh         ; did we read as many sectors as requested?
    jne sectors_error

    popa
    ret

disk_error:
    mov si, DISK_ERROR_MSG
    call print_string
    jmp disk_loop

sectors_error:
    mov si, SECTORS_ERROR_MSG
    call print_string

disk_loop:
    jmp $

DISK_ERROR_MSG:    db "Disk read error!", 0
SECTORS_ERROR_MSG: db "Incorrect number of sectors read!", 0
