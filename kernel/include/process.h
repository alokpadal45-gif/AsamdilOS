#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES        16
#define MAX_PAGES_PER_PROCESS 4
#define MAX_THREADS          12   /* max threads across all processes */

typedef enum {
    PROC_UNUSED = 0,
    PROC_READY,
    PROC_RUNNING,
    PROC_TERMINATED
} process_state_t;

typedef struct {
    int tid;                  /* thread ID */
    int owner_pid;            /* which process owns this thread */
    char name[32];
    process_state_t state;
    int burst_time;
    int remaining_time;
} thread_t;

typedef struct {
    int pid;
    char name[32];
    process_state_t state;
    int burst_time;
    int remaining_time;
    int pages[MAX_PAGES_PER_PROCESS];
    int page_count;
    int thread_count;
} process_t;

void  process_init(void);
int   process_create(const char *name, int burst_time);
void  process_spawn_system(const char *name, int burst_time, int pages);
int   process_kill(int pid);
void  process_list(void);
void  schedule_fcfs(void);
void  schedule_rr(int quantum);

/* multithreading */
int   thread_create(int pid, const char *name, int burst_time);
void  thread_list(int pid);
void  thread_schedule(void);   /* "threads" command - shows all threads running */

#endif
