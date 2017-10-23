#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/memory.h>

void page_init(Page *page, size_t num_pages, void *physbase, void *physfree) {
    Page *head = NULL, *prev = NULL;
    size_t i = 0;
    page[0].reference_count = 1;
    page[0].next = NULL;

    for (i = 1; i < (IOPHYSMEM / PAGE_SIZE); i++) {
        page[i].reference_count = 0;
        page[i].next = NULL;
        if (prev != NULL) {
            prev->next = &page[i];
        }
        if (head == NULL) {
            head = &page[i];
        }
        prev = &page[i];
    }

    for (; i < (((char *)physbase - (char *)EXTPHYSMEM) / PAGE_SIZE); i++) {
        page[i].reference_count = 0;
        page[i].next = NULL;
        if (prev != NULL) {
            prev->next = &page[i];
        }
        prev = &page[i];
    }

    for (; i < ((physfree - physbase) / PAGE_SIZE); i++) {
        page[i].reference_count = 1;
        page[i].next = NULL;
    }

    for (; i < num_pages; i++) {
        page[i].reference_count = 0;
        page[i].next = NULL;
        if (prev != NULL) {
            prev->next = &page[i];
        }
        prev = &page[i];
    }
}

void memory_init(void *physbase, void *physfree, size_t num_pages) {
    Page *page = (Page *)(KERNBASE + physfree);
    page_init(page, num_pages, physbase, physfree);
    kprintf("Done");
}
