# AsamdilOS - Report Outline

This is a writing guide, not the report itself - use it to write the
report in your own words, with screenshots from your own QEMU/VirtualBox
runs.

## 1. Introduction
- What AsamdilOS is: a 32-bit x86 OS built from a custom bootloader, C
  kernel, shell, file system, memory manager and process scheduler.
- Goals: demonstrate understanding of boot process, protected mode,
  memory management, scheduling algorithms, and file systems.
- Team roles: Alok Padal, Dilraj Bista, Samrat Karki (see table in README.md).
- Tools used: NASM, GCC (`-m32 -ffreestanding`), GNU LD, QEMU, VirtualBox.

## 2. System Architecture
- High-level diagram: BIOS -> Bootloader (16-bit) -> Protected mode switch
  -> Kernel (32-bit) -> Shell.
- Memory map: boot sector at `0x7C00`, kernel loaded at `0x1000`,
  stack at `0x90000`, VGA text buffer at `0xB8000`.
- Explain the build pipeline: `.asm`/`.c` -> object files -> linked flat
  binary (`linker.ld`) -> concatenated with the boot sector ->
  `os-image.bin`.

## 3. Bootloader Design
- 512-byte boot sector requirement and the `0xAA55` signature.
- Real-mode printing via BIOS `int 0x10` (`boot/print.asm`).
- Loading the kernel from disk with `int 0x13` (`boot/disk.asm`) -
  explain CHS addressing (cylinder/head/sector).
- The GDT (`boot/gdt.asm`): null/code/data descriptors, flat 4GB model.
- Enabling the A20 line and the switch to protected mode
  (`boot/switch_pm.asm`), including **why the far jump to reload CS is
  required** (this was the trickiest bug to get right!).

## 4. Kernel Design
- `kernel_entry.asm` as the entry point linked at `0x1000`.
- `kernel_main()` initialization order: memory -> processes -> file
  system -> login -> shell.
- VGA text-mode driver (`screen.c`): direct writes to `0xB8000`,
  hardware cursor via ports `0x3D4`/`0x3D5`, scrolling.
- Keyboard driver (`keyboard.c`): polling the PS/2 controller
  (port `0x64` status, port `0x60` data), scancode-to-ASCII translation,
  shift handling.
- System calls / service routines: in this design, kernel "services"
  (screen output, keyboard input, file/process operations) are exposed
  to the shell as ordinary C function calls within a single privilege
  level - explain this is a simplification vs. a real `int 0x80`-style
  syscall interface, and how it *could* be extended (a software
  interrupt + IDT entry could be added for a true syscall gate).

## 5. Memory Management
- `kmalloc()`: simple bump-pointer allocator over a static 64KB heap -
  explain why no `free()` is needed for this project's lifetime.
- Paging simulation (`palloc`/`pfree`): a bitmap of 1024 simulated
  4KB page frames (4MB total). Explain that this *models* the concept
  of physical page allocation without reprogramming the MMU/CR3,
  which keeps the kernel simple and stable.
- How processes "consume" pages based on their burst time, tying
  memory management to process management.
- The `meminfo` command output and what each number represents.

## 6. Process Scheduling
- The process table (`process_t`): PID, name, state, burst time,
  remaining time, allocated pages.
- `process_create`/`process_kill` and how memory pages are
  allocated/freed.
- **FCFS** (`schedule_fcfs`): processes run strictly in arrival order;
  show how waiting time and turnaround time are computed and printed.
- **Round Robin** (`schedule_rr`): fixed quantum (2 time units),
  processes cycle until their remaining time reaches zero; show the
  trace output as a Gantt-chart-style log.
- Discuss trade-offs: FCFS is simple but can cause long waits (convoy
  effect); RR gives fairer response time but adds context-switch
  overhead (simulated here as just bookkeeping).

## 7. File System
- In-memory table of up to 32 entries (`fs.c`), each either a file or
  a directory, with a fixed 256-byte content buffer.
- Commands: `mkdir`, `create`, `write`, `read`, `delete`/`rm`, `ls`.
- Limitations vs. a real file system: no persistence across reboots,
  flat namespace (no nested directories), fixed file size limit -
  discuss these as "Future Enhancements".

## 8. Shell Interface
- Login screen (`login_screen`): hardcoded credentials, password
  masking with `*`.
- Command loop: read a line, split into command + argument, dispatch
  via string comparison.
- Built-in utilities: `calc` (simple `+ - * /` parser), `date`
  (CMOS RTC reader), `sysinfo`, `meminfo`, `shutdown`.
- Error handling: unknown commands, missing arguments, division by
  zero, file-not-found, etc.

## 9. Testing
- Describe testing in QEMU (`make run`) and (optionally) VirtualBox.
- Walk through the demo command sequence from the README and include
  screenshots of:
  - Boot sequence and login screen
  - `help`, `about`, `date`, `calc 5+5`
  - File system commands (`mkdir`, `create`, `write`, `read`, `ls`)
  - `ps`, `run`, scheduler output, `kill`
  - `meminfo`, `sysinfo`
  - `shutdown`
- Note any edge cases you tested (e.g. `calc` with bad input,
  `read` on a missing file, `kill` on a non-existent PID).

## 10. Future Enhancements
- Real paging with CR3/page tables and virtual memory.
- Interrupt-driven (IDT/PIC) keyboard and a hardware timer for
  preemptive multitasking.
- Persistent storage (a real disk-backed file system, e.g. FAT12).
- True syscall interface via software interrupts and user/kernel
  privilege separation (ring 3 user mode).
- Nested directories and larger file sizes.
- A graphical mode (VESA/VGA graphics) instead of text mode.

---

## Marks alignment
This implementation includes a custom bootloader (not GRUB), a 32-bit
protected-mode C kernel, an interactive shell with login, an in-memory
file system, and simulated paging + FCFS/Round-Robin scheduling - all
running directly in QEMU/VirtualBox from `os-image.bin`, which targets
the **90-100%** "real bootable OS" tier described in the project brief.
