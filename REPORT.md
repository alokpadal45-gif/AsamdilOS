# AsamdilOS - Project Report

**Project:** AsamdilOS - A Custom 32-bit Operating System
**Team Members:** Alok Padal, Dilraj Bista, Samrat Karki

---

## 1. Introduction

For this project we decided to build our own operating system from
scratch instead of just simulating one in a high-level language. The
result is AsamdilOS, a small 32-bit x86 operating system that boots on
real hardware emulators like QEMU and VirtualBox using a bootloader we
wrote ourselves (no GRUB), a kernel written in C, an interactive shell
with a login screen, an in-memory file system, and a memory/process
manager that demonstrates paging and CPU scheduling.

Our main goal was to actually understand how an operating system comes
to life - starting from the moment the BIOS hands control over to our
code, all the way up to a working command prompt where a user can create
files, run processes and check system information.

We split the work into three areas, one per team member:

- **Alok Padal** - Bootloader & Kernel (boot process, screen output,
  keyboard input)
- **Dilraj Bista** - Memory & Process Management (heap allocator,
  paging simulation, process table, FCFS and Round Robin scheduling)
- **Samrat Karki** - File System & Shell (login, command shell, file
  system, calculator/date utilities)

The tools we used were NASM (assembler), GCC with `-m32 -ffreestanding`
(compiler), GNU `ld` (linker), and QEMU for testing. We also tried it in
VirtualBox to confirm it runs as a "real" OS and not just inside an
emulator.

---

## 2. System Architecture

At a high level, the boot process looks like this:

```
BIOS -> Bootloader (16-bit real mode) -> Protected mode switch
     -> Kernel (32-bit) -> Login -> Shell
```

When the computer (or QEMU) starts, the BIOS loads the first 512 bytes
of our disk image into memory at address `0x7C00` and jumps there. This
512-byte sector is our bootloader. It loads the rest of the kernel from
disk into memory at `0x1000`, switches the CPU from 16-bit real mode
into 32-bit protected mode, and then jumps into the kernel.

Important memory addresses in our OS:

| Address | What's there |
|---|---|
| `0x7C00` | Bootloader (loaded by BIOS) |
| `0x1000` | Kernel (loaded by our bootloader) |
| `0x90000` | Stack used in protected mode |
| `0xB8000` | VGA text video memory |

For the build process, each `.c` and `.asm` file is compiled/assembled
into object files, then all of them are linked together using our own
linker script (`linker.ld`) into a single flat binary called
`kernel.bin`. This is then glued onto the end of the 512-byte bootloader
(`boot.bin`) to produce the final file, `os-image.bin`, which is what
QEMU/VirtualBox actually boots from.

---

## 3. Bootloader Design

*(Alok Padal)*

The bootloader is the very first piece of code that runs, and it has a
hard requirement: it must fit in exactly 512 bytes and end with the
signature bytes `0x55 0xAA`, otherwise the BIOS won't even recognize it
as bootable.

Our bootloader does the following, in order:

1. Prints a startup message using BIOS interrupt `int 0x10` (teletype
   output) - this is real-mode printing, before we have our own screen
   driver.
2. Loads the kernel from disk into memory using `int 0x13` (BIOS disk
   read service). This works using CHS addressing - cylinder, head and
   sector numbers - to tell the BIOS which sectors to read. We load 40
   sectors (20KB), which is enough for our kernel.
3. Sets up a **Global Descriptor Table (GDT)** with three entries: a
   null descriptor (required), a code segment and a data segment, both
   set up as "flat" segments covering the whole 4GB address space.
4. Enables the **A20 line** (a legacy memory addressing quirk from the
   original IBM PC that has to be switched on to access memory above
   1MB).
5. Sets the **PE (Protection Enable) bit** in the CR0 register, which
   officially puts the CPU into 32-bit protected mode.
6. Jumps into the kernel at `0x1000`.

The trickiest part of this whole project was step 5 to 6. After setting
the PE bit, the CPU is technically in protected mode, but the CS
(code segment) register is still holding its old real-mode value, which
is no longer valid in protected mode. If you just do a normal `jmp` to
the 32-bit code, the very next instruction causes a **General Protection
Fault**, and because we don't have exception handlers set up yet, that
GP fault escalates into a double fault and then a **triple fault**,
which causes the CPU to reset completely. We spent a good amount of time
debugging this - the screen would just go back to the BIOS boot message
in a loop. The fix was to use a **far jump** that explicitly reloads CS
with our new code segment selector:

```asm
jmp dword CODE_SEG:BEGIN_PM
```

Once we changed this one line, the system booted cleanly into protected
mode every time.

---

## 4. Kernel Design

*(Alok Padal)*

Once the bootloader hands off control, the very first thing that runs is
`kernel_entry.asm`, a tiny assembly stub linked at address `0x1000`. Its
only job is to call `kernel_main()`, which is written in C.

`kernel_main()` runs the initialization steps in this order:

1. Clear the screen
2. Initialize the memory manager (`mem_init`)
3. Initialize the process table (`process_init`)
4. Initialize the file system (`fs_init`)
5. Show the login screen
6. Start the shell

**Screen output (`screen.c`)** is done by writing directly to VGA text
memory at address `0xB8000`. Each character on screen takes 2 bytes - one
byte for the ASCII character and one byte for its colour (foreground and
background). We also control the hardware text cursor through I/O ports
`0x3D4`/`0x3D5`, and handle scrolling manually - when the screen fills up,
we shift every row up by one and clear the last row.

**Keyboard input (`keyboard.c`)** uses a polling approach: we
continuously check the keyboard controller's status port (`0x64`) to see
if a key is waiting, and if so, read the scancode from port `0x60` and
translate it into an ASCII character using a lookup table. We also handle
the Shift key for uppercase letters and symbols.

We originally tried to do this with hardware interrupts (IRQs through the
PIC and an IDT), but this introduced its own set of bugs related to
interrupt gates and General Protection Faults, similar to the one
described above. Since a polling keyboard driver is simpler, more
reliable, and still feels completely responsive in QEMU, we went with
that approach instead.

**System calls:** in a "real" OS, user programs request kernel services
(like printing to the screen or reading a file) through a software
interrupt, e.g. `int 0x80` on Linux, which switches the CPU to kernel
privilege level. In AsamdilOS, everything runs at the same privilege
level, so our "system calls" are simply C function calls from the shell
into the kernel's screen, keyboard, file system and process management
modules. This is a simplification we made on purpose to keep the project
manageable, but the same software-interrupt mechanism we got working for
the bootloader's protected-mode switch could, in principle, be extended
into a proper `int 0x80`-style syscall gate with its own IDT entry.

---

## 5. Memory Management

*(Dilraj Bista)*

For memory management we implemented two separate but related systems:
a heap allocator and a page allocation simulation.

**Heap allocator (`kmalloc`)** - This is a very simple "bump pointer"
allocator. We reserve a 64KB array as our kernel heap, and every time
`kmalloc(size)` is called, we hand out the next free chunk and move the
pointer forward. We deliberately did not implement `kfree()`, because
within the lifetime of this kernel (it never "exits" a program and
reclaims memory the way a real OS would), nothing ever needs to be freed
from the heap - it's allocated once during initialisation and stays in
use.

**Paging simulation (`palloc` / `pfree`)** - Real paging involves setting
up page tables and reprogramming the CR3 register so the CPU's memory
management unit translates virtual addresses to physical ones. We decided
not to implement this for real, because a mistake here (similar to the
GDT/protected-mode issue described earlier) can easily crash the whole
system, and debugging page faults without proper exception handlers would
have been extremely time-consuming.

Instead, we built a **page allocation bitmap**: an array of 1024 entries,
each representing a 4KB "page" (so 4MB total simulated memory). Each
entry is either 0 (free) or 1 (used). `palloc()` finds the first free
entry, marks it used, and returns its index. `pfree()` clears that bit
again. This models the *concept* of page-based memory allocation -
processes requesting and releasing fixed-size chunks of memory - without
needing to touch the actual hardware MMU.

**Connecting memory to processes** - when a new process is created, we
calculate how many pages it "needs" based on its burst time (one page for
every 2 units of burst time, minimum 1), and call `palloc()` that many
times. When the process is killed, we call `pfree()` for each of its
pages. This means the `meminfo` command's numbers actually change as you
create and kill processes, which we thought was a nice way to tie the two
subsystems together.

**`meminfo` output** shows:
- How much of the 64KB kernel heap has been used
- How many of the 1024 simulated page frames are in use
- The simulated total/used/free RAM in KB, calculated from the page
  count

---

## 6. Process Scheduling

*(Dilraj Bista)*

We maintain a simple process table (`process_t`) with up to 16 entries.
Each entry stores a process ID (PID), a name, its current state
(`READY`, `RUNNING`, `TERMINATED` or `UNUSED`), its burst time (how much
"CPU time" it needs), its remaining time (used during Round Robin), and
the list of memory pages it owns.

`process_create(name, burst_time)` finds a free slot in the table,
assigns it the next PID, sets its state to `READY`, and allocates pages
as described in the Memory Management section. `process_kill(pid)` frees
those pages and marks the slot as unused again.

We implemented two scheduling algorithms, both triggered by the `run`
command:

**First Come First Served (FCFS)** - `schedule_fcfs()` simply walks
through the process table in order and "runs" each ready process for its
full burst time, one after another. For each process we print the time it
starts, the time it finishes, and its waiting time (which for FCFS is
just the sum of the burst times of all processes before it). At the end
we print the total turnaround time.

**Round Robin (RR)** - `schedule_rr(quantum)` runs with a fixed time
quantum of 2 units. It loops through all ready processes giving each one
a maximum of `quantum` units of CPU time per turn, reducing its remaining
time each round, until every process's remaining time reaches zero. The
output looks like a simple Gantt chart, e.g.:

```
t=0: PID 1 (test) runs for 2 unit(s)
t=2: PID 1 (test) runs for 2 unit(s)
t=4: PID 1 (test) runs for 2 unit(s)
 -> PID 1 finished at t=6
```

**Discussion / trade-offs** - FCFS is the simplest possible scheduler,
but it suffers from the "convoy effect": if a long process arrives first,
every shorter process behind it has to wait a long time even though they
individually need very little CPU time. Round Robin fixes this by giving
every process a fair, small slice of time in turn, which improves average
response time - but in a real system this comes at the cost of context
switching overhead (saving and restoring CPU registers every time you
switch processes), which we represent here only as simple bookkeeping
since we aren't doing real preemptive multitasking.

`ps` lets you see the current state of the process table at any time -
PID, name, state, burst time and number of allocated pages - which is
useful for checking that `run` and `kill` are actually changing things.

---

## 7. File System

*(Samrat Karki)*

AsamdilOS includes a small **in-memory file system** - there's no real
disk storage involved, everything lives in a fixed-size table in RAM
(which means it resets every time the OS reboots, but it's enough to
demonstrate how file operations work).

The table holds up to 32 entries. Each entry has:
- A name (up to 32 characters)
- A type: either a file or a directory
- A content buffer (up to 256 bytes, for files)
- A size field

The commands that operate on this table are:

- `mkdir <name>` - creates a directory entry
- `create <name>` - creates an empty file
- `write <name> <text>` - writes text into an existing file (overwrites
  previous content)
- `read <name>` - prints a file's contents
- `delete <name>` / `rm <name>` - removes a file or directory entry
- `ls` - lists everything currently in the file system, showing `[DIR]`
  or `[FILE]` and the size in bytes for files

**Limitations** - this is obviously a simplified file system compared to
something like FAT or ext4. The big ones are: nothing is saved to disk
(it's wiped on reboot), there's no real folder nesting (directories are
just markers in the same flat list, not actual containers), and each
file is capped at 256 bytes. We talk about how these could be improved in
the Future Enhancements section.

---

## 8. Shell Interface

*(Samrat Karki)*

The shell is the main way a user interacts with AsamdilOS, and it starts
with a **login screen**. The username and password are currently
hardcoded (`admin` / `1234`) in `shell.c`. While typing the password, the
screen shows `*` characters instead of the actual letters, so it isn't
displayed in plain text.

Once logged in, the shell runs a loop:
1. Print a prompt (`AsamdilOS> `)
2. Read a line of input from the keyboard
3. Split it into a command and an argument (everything after the first
   space)
4. Compare the command against our list of known commands and call the
   matching function

Some of the "utility" commands we built:
- `calc <a><op><b>` - a basic calculator that supports `+`, `-`, `*` and
  `/` between two integers, e.g. `calc 5+5` gives `5+5 = 10`
- `date` - reads the current date and time directly from the computer's
  CMOS real-time clock chip (the same chip your motherboard uses to keep
  time even when the PC is off)
- `sysinfo` - prints the OS name, version, architecture, and a memory
  summary
- `meminfo` - prints heap and page allocation details
- `shutdown` - halts the CPU

**Error handling** - we made sure the shell doesn't crash on bad input.
For example:
- `calc` with no operator (e.g. `calc 55`) prints an error instead of
  guessing
- `calc 5/0` is caught and reports a division-by-zero error rather than
  crashing
- `read`, `delete`, `kill` etc. on a name/PID that doesn't exist print a
  clear "no such file" / "no such process" message
- Typing a completely unknown command tells the user to type `help`

---

## 9. Testing

We tested AsamdilOS primarily in **QEMU**, using the command:

```
qemu-system-i386 -drive format=raw,file=os-image.bin -display curses
```

(We initially tried running QEMU with a graphical window through WSL,
but had display issues on our setup, so we switched to the `curses`
text-mode display, which works reliably and is actually easier to take
clean screenshots of for this report.)

Our testing process was to go through the full command list from start
to finish and check the output of each one:

- Boot sequence and login screen - confirms the bootloader, protected
  mode switch, and kernel initialisation all work
- `help`, `about`, `date`, `calc 5+5` - basic shell utilities
- `mkdir docs`, `create notes.txt`, `write notes.txt ...`, `read
  notes.txt`, `ls` - full file system workflow
- `run test`, `ps`, scheduler output, `kill 1` - process creation,
  scheduling and termination
- `meminfo`, `sysinfo` - memory/system reporting
- `shutdown` - clean halt

**Edge cases we specifically tested:**
- `calc` with no valid operator (e.g. `calc 55`) - correctly shows an
  error
- `calc 5/0` - correctly shows a division-by-zero error instead of
  crashing
- `read` on a file that doesn't exist - shows "no such file"
- `kill` on a PID that doesn't exist - shows "no such process"
- Running `ps` and `meminfo` before and after `run`/`kill` to confirm the
  process table and page counts update correctly

We also found and fixed one real bug during testing: our screen-scrolling
code had an off-by-one issue where, once the screen filled up and started
scrolling, the cursor position could drift past the end of video memory.
This caused the screen to go completely blank after enough output (we
first noticed this when running `run test`, since the FCFS/RR scheduler
output was enough text to trigger it). We fixed it by making sure the
scroll function's corrected cursor position is actually used afterwards,
instead of being overwritten by the old, out-of-range value.

---

## 10. Future Enhancements

If we had more time, here's what we'd want to add next:

- **Real paging** - actual page tables and CR3 register setup, giving
  processes their own virtual address spaces instead of just simulating
  page counts
- **Interrupt-driven I/O** - a proper IDT/PIC setup so the keyboard
  doesn't need to be polled, plus a hardware timer (PIT) to enable real
  preemptive multitasking instead of our "simulate one process at a time"
  scheduler demos
- **Persistent storage** - a real disk-backed file system (e.g. FAT12),
  so files survive a reboot
- **A real syscall interface** - using a software interrupt (like
  `int 0x80`) and separating user programs from the kernel with proper
  ring 3 / ring 0 privilege levels
- **Proper nested directories** and larger file sizes in the file system
- **A graphics mode** instead of text mode, using VESA/VGA graphics for a
  more visual interface

---

## Conclusion

Building AsamdilOS taught us a lot about what's actually happening "under
the hood" of a computer before any operating system we normally use even
starts - from BIOS interrupts and CHS disk addressing, to the GDT and
protected mode, to writing our own drivers and shell from nothing. The
hardest and most educational part was definitely debugging the protected
mode switch, since a single missing instruction caused the entire system
to silently reboot in a loop with no error message. Getting past that and
seeing our own login screen and shell come up for the first time was
genuinely satisfying.

The final result - AsamdilOS - is a custom bootloader, a 32-bit C kernel,
a login-protected shell, an in-memory file system, and working FCFS and
Round Robin scheduling demos, all running directly from our own
`os-image.bin` in QEMU.
