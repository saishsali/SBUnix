#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/memory.h>

Page *page, *page_free_list;

void page_init(uint64_t start, uint64_t end, void *physbase, void *physfree) {
    Page *prev = NULL;
    uint64_t i = 0;
    static uint64_t index = 0;

    /* Mark page index to start as in use (holes) so that it can never be allocated */
    for (i = index; i < start; i++) {
        page[i].reference_count = 1;
        page[i].next = NULL;
    }

    /* Mark page 0 as in use */
    if (start == 0) {
        page[0].reference_count = 1;
        page[0].next = NULL;
        start += 1;
    }

    for (i = start; i < end; i++) {
        /* Mark kernel memory as in use */
        if (i >= ((uint64_t)physbase / PAGE_SIZE) && i < ((uint64_t)physfree / PAGE_SIZE)) {
            page[i].reference_count = 1;
            page[i].next = NULL;
        } else {
            /* Mark rest of the memory as free */
            page[i].reference_count = 0;
            page[i].next = NULL;
            if (prev != NULL) {
                prev->next = &page[i];
            }
            if (page_free_list == NULL) {
                page_free_list = &page[i];
            }
            prev = &page[i];
        }
    }
    index = end;
}

void memory_init(uint32_t *modulep, void *physbase, void *physfree) {
    int i;
    page = (Page *)(KERNBASE + physfree);
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    while (modulep[0] != 0x9001) modulep += modulep[1] + 2;
    for (smap = (struct smap_t*)(modulep + 2); smap < (struct smap_t*)((char*)modulep+modulep[1] + 2 * 4); ++smap) {
        if (smap->type == 1 && smap->length != 0) {
            page_init(smap->base / PAGE_SIZE, (smap->base + smap->length) / PAGE_SIZE, physbase, physfree);
        }
    }

    for (i = 510; i < 550; i++) {
        kprintf("Page %d: %d ", i, page_free_list[i].reference_count);
    }
}
