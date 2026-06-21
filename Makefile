# =======================================================
# AsamdilOS Makefile
#
# Builds:
#   boot.bin        - 512 byte boot sector (custom bootloader)
#   kernel.bin       - flat binary kernel image
#   os-image.bin     - boot.bin + kernel.bin, the final
#                       bootable disk image for QEMU/VirtualBox
#
# Usage:
#   make        - build os-image.bin
#   make run    - build and run in QEMU
#   make clean  - remove build artifacts
# =======================================================

CC   = gcc
LD   = ld
NASM = nasm

CFLAGS  = -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector \
          -nostdlib -Wall -Wextra -Ikernel/include -c
LDFLAGS = -m elf_i386 -T linker.ld

C_SOURCES   = $(wildcard kernel/*.c)
C_OBJECTS   = $(C_SOURCES:.c=.o)
ASM_OBJECTS = kernel/kernel_entry.o

.PHONY: all run clean

all: os-image.bin

run: os-image.bin
	qemu-system-i386 -drive format=raw,file=os-image.bin

os-image.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.bin
	truncate -s 32768 os-image.bin
	@echo "Built os-image.bin ($$(stat -c%s os-image.bin) bytes)"

boot.bin: boot/boot.asm boot/print.asm boot/disk.asm boot/gdt.asm boot/switch_pm.asm boot/print_pm.asm
	$(NASM) -f bin boot/boot.asm -o boot.bin

kernel.bin: kernel_full.elf
	objcopy -O binary kernel_full.elf kernel.bin

kernel_full.elf: $(ASM_OBJECTS) $(C_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(ASM_OBJECTS) $(C_OBJECTS)

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) $< -o $@

kernel/%.o: kernel/%.asm
	$(NASM) -f elf32 $< -o $@

clean:
	rm -f boot.bin kernel.bin kernel_full.elf os-image.bin kernel/*.o
