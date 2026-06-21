/* =======================================================
 * process.c
 * Dilraj Bista - Memory & Process Management
 *
 * Maintains a simple process table and demonstrates two
 * classic CPU scheduling algorithms - First Come First
 * Served (FCFS) and Round Robin (RR) - by simulating their
 * execution order and printing a trace + summary statistics.
 *
 * Memory for each "process" is requested from the page
 * allocator in memory.c, tying process management into the
 * paging simulation.
 * ======================================================= */

#include "process.h"
#include "memory.h"
#include "screen.h"
#include "string.h"

static process_t processes[MAX_PROCESSES];
static int next_pid = 1;

static thread_t threads[MAX_THREADS];
static int next_tid = 1;

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = PROC_UNUSED;
        processes[i].page_count = 0;
        processes[i].thread_count = 0;
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        threads[i].state = PROC_UNUSED;
    }
    next_pid = 1;
    next_tid = 1;
}

/* Spawns a built-in kernel/system process that is marked as RUNNING
 * from the moment the OS boots - just like init, idle, klogd etc.
 * on a real Linux system. These show up in "ps" automatically. */
void process_spawn_system(const char *name, int burst_time, int pages) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROC_UNUSED) { slot = i; break; }
    }
    if (slot == -1) return;

    process_t *p = &processes[slot];
    p->pid = next_pid++;
    strcpy(p->name, name);
    p->state    = PROC_RUNNING;   /* already running at boot */
    p->burst_time    = burst_time;
    p->remaining_time = burst_time;
    p->page_count = pages;
    for (int i = 0; i < pages; i++) {
        p->pages[i] = palloc();
    }
}

int process_create(const char *name, int burst_time) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROC_UNUSED) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        screen_print_color("Error: process table full\n", RED_ON_BLACK);
        return -1;
    }

    process_t *p = &processes[slot];
    p->pid = next_pid++;
    strcpy(p->name, name);
    p->state = PROC_READY;
    p->burst_time = burst_time;
    p->remaining_time = burst_time;

    /* Simulate allocating memory pages for this process:
     * one page per 2 units of burst time, at least 1 page. */
    p->page_count = (burst_time / 2) + 1;
    if (p->page_count > MAX_PAGES_PER_PROCESS) p->page_count = MAX_PAGES_PER_PROCESS;

    for (int i = 0; i < p->page_count; i++) {
        int page = palloc();
        p->pages[i] = page; /* may be -1 if memory is exhausted */
    }

    screen_print("Created process PID ");
    screen_print_dec(p->pid);
    screen_print(" (");
    screen_print(p->name);
    screen_print(") - burst=");
    screen_print_dec(burst_time);
    screen_print(", pages=");
    screen_print_dec(p->page_count);
    screen_print("\n");

    return p->pid;
}

static process_t *find_by_pid(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROC_UNUSED && processes[i].pid == pid) {
            return &processes[i];
        }
    }
    return NULL;
}

int process_kill(int pid) {
    process_t *p = find_by_pid(pid);
    if (!p) {
        screen_print_color("Error: no such process\n", RED_ON_BLACK);
        return -1;
    }

    for (int i = 0; i < p->page_count; i++) {
        if (p->pages[i] >= 0) pfree(p->pages[i]);
    }

    p->state = PROC_UNUSED;
    p->page_count = 0;

    screen_print("Killed process PID ");
    screen_print_dec(pid);
    screen_print("\n");
    return 0;
}

static const char *state_name(process_state_t s) {
    switch (s) {
        case PROC_READY:      return "READY";
        case PROC_RUNNING:    return "RUNNING";
        case PROC_TERMINATED: return "TERMINATED";
        default:              return "UNUSED";
    }
}

void process_list(void) {
    screen_print_color("PID  NAME             STATE        BURST  PAGES\n", CYAN_ON_BLACK);

    int any = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROC_UNUSED) continue;
        any = 1;
        process_t *p = &processes[i];

        screen_print_dec(p->pid);
        screen_print("    ");
        screen_print(p->name);

        /* pad name to roughly align columns */
        int pad = 17 - (int)strlen(p->name);
        for (int j = 0; j < pad; j++) screen_print(" ");

        screen_print(state_name(p->state));

        pad = 13 - (int)strlen(state_name(p->state));
        for (int j = 0; j < pad; j++) screen_print(" ");

        screen_print_dec(p->burst_time);
        screen_print("      ");
        screen_print_dec(p->page_count);
        screen_print("\n");
    }

    if (!any) {
        screen_print("(no active processes)\n");
    }
}

/* ---- First Come First Served scheduling demo ---- */
void schedule_fcfs(void) {
    screen_print_color("=== FCFS Scheduling ===\n", CYAN_ON_BLACK);

    int time = 0;
    int any = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_t *p = &processes[i];
        if (p->state != PROC_READY) continue;
        any = 1;

        int start = time;
        int finish = start + p->burst_time;

        screen_print("PID ");
        screen_print_dec(p->pid);
        screen_print(" (");
        screen_print(p->name);
        screen_print(") runs from t=");
        screen_print_dec(start);
        screen_print(" to t=");
        screen_print_dec(finish);
        screen_print(" (waiting time=");
        screen_print_dec(start);
        screen_print(")\n");

        time = finish;
        p->remaining_time = 0;
    }

    if (!any) {
        screen_print("(no ready processes to schedule)\n");
        return;
    }

    screen_print("Total turnaround time: ");
    screen_print_dec(time);
    screen_print("\n");
}

/* ---- Round Robin scheduling demo ---- */
void schedule_rr(int quantum) {
    screen_print_color("=== Round Robin Scheduling (quantum=", CYAN_ON_BLACK);
    screen_print_dec(quantum);
    screen_print_color(") ===\n", CYAN_ON_BLACK);

    /* reset remaining time for all ready processes */
    int any = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROC_READY) {
            processes[i].remaining_time = processes[i].burst_time;
            any = 1;
        }
    }

    if (!any) {
        screen_print("(no ready processes to schedule)\n");
        return;
    }

    int time = 0;
    int done;

    do {
        done = 1;
        for (int i = 0; i < MAX_PROCESSES; i++) {
            process_t *p = &processes[i];
            if (p->state != PROC_READY) continue;
            if (p->remaining_time <= 0) continue;

            done = 0;
            int slice = (p->remaining_time < quantum) ? p->remaining_time : quantum;

            screen_print("t=");
            screen_print_dec(time);
            screen_print(": PID ");
            screen_print_dec(p->pid);
            screen_print(" (");
            screen_print(p->name);
            screen_print(") runs for ");
            screen_print_dec(slice);
            screen_print(" unit(s)\n");

            time += slice;
            p->remaining_time -= slice;

            if (p->remaining_time == 0) {
                screen_print("  -> PID ");
                screen_print_dec(p->pid);
                screen_print(" finished at t=");
                screen_print_dec(time);
                screen_print("\n");
            }
        }
    } while (!done);

    screen_print("Total turnaround time: ");
    screen_print_dec(time);
    screen_print("\n");
}

/* ======================================================= 
 * Multithreading simulation
 * Dilraj Bista - Memory & Process Management
 * ======================================================= */

int thread_create(int pid, const char *name, int burst_time) {
    int slot = -1;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == PROC_UNUSED) { slot = i; break; }
    }
    if (slot == -1) {
        screen_print_color("Error: max threads reached\n", RED_ON_BLACK);
        return -1;
    }
    threads[slot].tid            = next_tid++;
    threads[slot].owner_pid      = pid;
    strcpy(threads[slot].name, name);
    threads[slot].state          = PROC_READY;
    threads[slot].burst_time     = burst_time;
    threads[slot].remaining_time = burst_time;
    screen_print("Thread TID ");
    screen_print_dec(threads[slot].tid);
    screen_print(" (");
    screen_print(name);
    screen_print(") created under PID ");
    screen_print_dec(pid);
    screen_print("\n");
    return threads[slot].tid;
}

void thread_list(int pid) {
    screen_print_color("TID  OWNER  NAME             STATE        BURST\n", CYAN_ON_BLACK);
    int any = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == PROC_UNUSED) continue;
        if (pid != -1 && threads[i].owner_pid != pid) continue;
        any = 1;
        screen_print_dec(threads[i].tid);
        screen_print("    ");
        screen_print_dec(threads[i].owner_pid);
        screen_print("      ");
        screen_print(threads[i].name);
        int pad = 17 - (int)strlen(threads[i].name);
        for (int j = 0; j < pad; j++) screen_print(" ");
        if (threads[i].state == PROC_READY)        screen_print("READY        ");
        else if (threads[i].state == PROC_RUNNING)  screen_print("RUNNING      ");
        else                                         screen_print("DONE         ");
        screen_print_dec(threads[i].burst_time);
        screen_print("\n");
    }
    if (!any) screen_print("(no threads - use: thread <pid> <name>)\n");
}

void thread_schedule(void) {
    int any = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == PROC_READY && threads[i].remaining_time > 0) {
            any = 1; break;
        }
    }
    if (!any) {
        screen_print("No threads. Use: thread <pid> <name>\n");
        return;
    }
    screen_print_color("\n=== Thread Scheduling - Round Robin (quantum=1) ===\n", CYAN_ON_BLACK);
    int time = 0, done;
    do {
        done = 1;
        for (int i = 0; i < MAX_THREADS; i++) {
            if (threads[i].state != PROC_READY) continue;
            if (threads[i].remaining_time <= 0)  continue;
            done = 0;
            screen_print_color("[CPU] ", GREEN_ON_BLACK);
            screen_print("t=");
            screen_print_dec(time);
            screen_print(": TID ");
            screen_print_dec(threads[i].tid);
            screen_print(" (");
            screen_print(threads[i].name);
            screen_print(") runs 1 unit [left=");
            screen_print_dec(threads[i].remaining_time - 1);
            screen_print("]\n");
            time++;
            threads[i].remaining_time--;
            if (threads[i].remaining_time == 0) {
                screen_print_color("[DONE] TID ", YELLOW_ON_BLACK);
                screen_print_dec(threads[i].tid);
                screen_print(" finished at t=");
                screen_print_dec(time);
                screen_print("\n");
                threads[i].state = PROC_TERMINATED;
            }
        }
    } while (!done);
    screen_print("All threads done. Total time = ");
    screen_print_dec(time);
    screen_print("\n");
    screen_print_color("(Real OS uses hardware timer to preempt threads)\n", YELLOW_ON_BLACK);
}
