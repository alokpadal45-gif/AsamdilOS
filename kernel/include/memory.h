#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE      4096
#define TOTAL_PAGES    1024          /* simulated 4MB managed by the "paging" layer */
#define HEAP_SIZE      (64 * 1024)   /* 64KB kernel heap for kmalloc */

void  mem_init(void);
void *kmalloc(u32 size);

/* Simple page allocator used to simulate paging.
 * Returns the page index (0..TOTAL_PAGES-1) or -1 if full. */
int  palloc(void);
void pfree(int page_index);

/* Prints memory usage information for the "meminfo" command */
void mem_print_info(void);

#endif
