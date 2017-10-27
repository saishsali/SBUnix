#ifndef _MEMORY_H
#define _MEMORY_H

/* Page size is 4KB */
#define PAGE_SIZE 4096

struct Page {
    /* Next page on free list */
    struct Page *next;
    /* Count of pointers referring to this page */
    uint16_t reference_count;
}__attribute__((packed));

typedef struct Page Page;

void page_init(uint64_t start, uint64_t end, uint64_t physbase, uint64_t physfree);

#endif
