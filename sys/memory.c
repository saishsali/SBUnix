#include <sys/kprintf.h>
#include <sys/defs.h>
#define PAGE_SIZE 4096

struct page_descriptor {
    struct page_descriptor *next;
    struct page_descriptor *previous;
}__attribute__((packed));

void create_page_descriptor(uint32_t *modulep, void *physbase, void *physfree) {
    uint64_t num_pages = 0;
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    while (modulep[0] != 0x9001) modulep += modulep[1] + 2;
    for(smap = (struct smap_t*)(modulep + 2); smap < (struct smap_t*)((char*)modulep+modulep[1] + 2 * 4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            num_pages += smap->length/PAGE_SIZE;
            kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
        }
    }
    kprintf("Number of pages: %d\n", num_pages);
}
