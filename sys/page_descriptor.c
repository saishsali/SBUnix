#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/page_descriptor.h>

Page *page_free_list;

void page_init(uint64_t start, uint64_t end, uint64_t physbase, uint64_t physfree) {
    static uint64_t index = 0;
    /* Start page descriptor array at physfree */
    Page *page = (Page *)physfree;
    Page *prev = NULL;
    uint64_t i = 0;
    start = start / PAGE_SIZE;
    end = end / PAGE_SIZE;

    /* Mark page index to start as in use so that it can never be allocated */
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
        if (i >= (physbase / PAGE_SIZE) && i < (physfree / PAGE_SIZE)) {
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
