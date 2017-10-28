#ifndef _PAGE_DESCRIPTOR_H
#define _PAGE_DESCRIPTOR_H

/* Page size is 4KB */
#define PAGE_SIZE  4096
#define PAGE_SHIFT 12

/* Kernbase: refer to linker.script */
#define KERNBASE 0xffffffff80000000

struct Page {
    /* Next page on free list */
    struct Page *next;
    /* Track the number of active mappings */
    uint16_t reference_count;
}__attribute__((packed));

typedef struct Page Page;

void page_init(uint64_t start, uint64_t end, uint64_t physbase, uint64_t physfree);
Page *allocate_page();
Page *allocate_pages(int num_pages);
uint64_t page_to_physical_address(Page *p);

#endif
