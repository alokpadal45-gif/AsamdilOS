# AsamdilOS

A small 32-bit x86 operating system built from scratch with a custom bootloader, a C kernel, an interactive command shell, an in-memory file system, and simulated memory paging and CPU scheduling.

## Team & Responsibilities

| Student | Area | Files |
|---|---|---|
| **Alok Padal** | Bootloader & Kernel | `boot/*.asm`, `kernel/kernel_entry.asm`, `kernel/kernel.c`, `kernel/screen.c`, `kernel/keyboard.c` |
| **Dilraj Bista** | Memory & Process Management | `kernel/memory.c`, `kernel/process.c` |
| **Samrat Karki** | File System & Shell | `kernel/fs.c`, `kernel/shell.c`, `kernel/rtc.c` |

> `kernel/string.c` (helper string/memory functions) is shared by everyone.

## Requirements

Linux is strongly recommended. WSL2 on Windows works perfectly too.

Install the following tools:

```bash
sudo apt update
sudo apt install -y build-essential nasm qemu-system-x86 gcc-multilib
```

| Tool | Purpose |
|---|---|
| `nasm` | Assembles the bootloader and kernel entry stub |
| `gcc-multilib` | Lets gcc compile 32-bit (`-m32`) code on a 64-bit machine |
| `qemu-system-x86` | Runs the finished OS image |

## Building and Running

Open this folder in VS Code (`File > Open Folder...`).

### From the Terminal

```bash
make        # Builds os-image.bin
make run    # Builds (if needed) and launches QEMU
make clean  # Removes all build artifacts
```

### From VS Code

Press **Ctrl+Shift+B** (or `Terminal > Run Task...`) and choose:

- **Build AsamdilOS** — compiles everything into `os-image.bin`
- **Run AsamdilOS in QEMU** — builds and boots the OS in a QEMU window

## Running in VirtualBox (optional)

1. Run `make` to produce `os-image.bin`
2. Convert it to a VDI disk image:

```bash
VBoxManage convertfromraw os-image.bin os-image.vdi --format VDI
```

3. In VirtualBox, create a new VM:
   - **Type:** Other, **Version:** Other/Unknown (32-bit)
   - Attach `os-image.vdi` as the primary hard disk
4. Boot the VM — AsamdilOS will start

## Project Structure

```
AsamdilOS/
├── boot/
│   └── *.asm              # Bootloader (16-bit real mode → protected mode)
├── kernel/
│   ├── kernel_entry.asm   # Entry stub, sets up stack and calls kernel_main
│   ├── kernel.c           # Main kernel: init and event loop
│   ├── screen.c           # VGA text-mode driver
│   ├── keyboard.c         # PS/2 keyboard ISR and scancode map
│   ├── memory.c           # Page-frame allocator and virtual memory
│   ├── process.c          # Process table and round-robin scheduler
│   ├── fs.c               # In-memory flat file system
│   ├── shell.c            # Interactive command shell
│   ├── rtc.c              # Real-time clock driver
│   └── string.c           # Shared string/memory helpers
├── Makefile
└── README.md
```

## Features

- Custom bootloader that transitions the CPU from 16-bit real mode to 32-bit protected mode
- Bare-metal C kernel with no standard library
- VGA text-mode display driver
- Interrupt-driven PS/2 keyboard driver
- Simulated memory paging and page-frame allocator
- Round-robin CPU scheduler
- In-memory flat file system
- Interactive command shell
- RTC driver for reading timestamps

## License

This project was made for educational purposes.