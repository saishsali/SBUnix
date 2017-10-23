#ifndef _MEMORY_H
#define _MEMORY_H

/* Physical memory is mapped to this address (Ref: linker.script) */
#define KERNBASE 0xffffffff80000000

/* At IOPHYSMEM (640K) there is a 384K hole for I/O.  From the kernel,
IOPHYSMEM can be addressed at KERNBASE + IOPHYSMEM.  The hole ends
at physical address EXTPHYSMEM. */
#define IOPHYSMEM   0x0A0000
#define EXTPHYSMEM  0x100000

/* Page size is 4KB */
#define PAGE_SIZE 4096

struct Page {
    /* Next page on free list */
    struct Page *next;
    /* Count of pointers referring to this page */
    uint16_t reference_count;
}__attribute__((packed));

typedef struct Page Page;

void memory_init(uint32_t *modulep, void *physbase, void *physfree);

#endif
