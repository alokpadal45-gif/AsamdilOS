/* =======================================================
 * memory.c
 * Dilraj Bista - Memory & Process Management
 *
 * Provides:
 *  - kmalloc(): a simple bump allocator over a static heap
 *    (no kfree - acceptable for the lifetime of this
 *    educational kernel, which never tears anything down)
 *  - A page allocation bitmap that simulates paging: the
 *    "physical memory" is divided into TOTAL_PAGES pages of
 *    PAGE_SIZE bytes, and processes "request" pages from
 *    this pool. This models the concept of paging without
 *    needing to reprogram CR3 / the MMU.
 * ======================================================= */

#include "memory.h"
#include "screen.h"
#include "string.h"

/* --- simple bump-pointer heap --- */
static u8 heap[HEAP_SIZE];
static u32 heap_used = 0;

void mem_init(void) {
    heap_used = 0;
    memset(heap, 0, HEAP_SIZE);
}

void *kmalloc(u32 size) {
    /* align to 4 bytes */
    if (size % 4 != 0) size += 4 - (size % 4);

    if (heap_used + size > HEAP_SIZE) {
        return NULL; /* out of memory */
    }

    void *ptr = &heap[heap_used];
    heap_used += size;
    return ptr;
}

/* --- page allocation bitmap (paging simulation) --- */
static u8 page_bitmap[TOTAL_PAGES]; /* 0 = free, 1 = used */
static u32 pages_used_count = 0;

int palloc(void) {
    for (int i = 0; i < TOTAL_PAGES; i++) {
        if (page_bitmap[i] == 0) {
            page_bitmap[i] = 1;
            pages_used_count++;
            return i;
        }
    }
    return -1; /* no free pages */
}

void pfree(int page_index) {
    if (page_index < 0 || page_index >= TOTAL_PAGES) return;
    if (page_bitmap[page_index] == 1) {
        page_bitmap[page_index] = 0;
        pages_used_count--;
    }
}

void mem_print_info(void) {
    screen_print_color("=== Memory Information ===\n", CYAN_ON_BLACK);

    screen_print("Kernel heap : ");
    screen_print_dec(heap_used);
    screen_print(" / ");
    screen_print_dec(HEAP_SIZE);
    screen_print(" bytes used\n");

    screen_print("Page frames : ");
    screen_print_dec(pages_used_count);
    screen_print(" / ");
    screen_print_dec(TOTAL_PAGES);
    screen_print(" pages used (");
    screen_print_dec(PAGE_SIZE);
    screen_print(" bytes each)\n");

    u32 total_bytes = TOTAL_PAGES * PAGE_SIZE;
    u32 used_bytes  = pages_used_count * PAGE_SIZE;
    u32 free_bytes  = total_bytes - used_bytes;

    screen_print("Simulated RAM: ");
    screen_print_dec(total_bytes / 1024);
    screen_print(" KB total, ");
    screen_print_dec(used_bytes / 1024);
    screen_print(" KB used, ");
    screen_print_dec(free_bytes / 1024);
    screen_print(" KB free\n");
}
