#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/page_descriptor.h>
#include <sys/kprintf.h>

Page *page_free_list, *pages;

uint64_t page_to_physical_address(Page *p) {
    /* Offset * 4096 */
    return (p - pages) << PAGE_SHIFT;
}

uint64_t page_to_virtual_address(Page *p) {
    return KERNBASE + page_to_physical_address(p);
}

void page_init(uint64_t start, uint64_t end, uint64_t physbase, uint64_t physfree) {
    static uint64_t index = 0;

    /* Start page descriptor array at KERBASE + physfree */
    pages = (Page *)(KERNBASE + physfree);
    Page *prev = NULL;
    uint64_t i = 0;
    start = start / PAGE_SIZE;
    end = end / PAGE_SIZE;

    /* Mark page index to start as in use so that it can never be allocated */
    for (i = index; i < start; i++) {
        pages[i].reference_count = 1;
        pages[i].next = NULL;
    }

    /* Mark page 0 as in use */
    if (start == 0) {
        pages[0].reference_count = 1;
        pages[0].next = NULL;
        start++;
    }

    for (i = start; i < end; i++) {
        /* Mark memory until physfree as free but do not initialize page_free_list (Since bootloader page table exists in this region)*/
        if (i < (physfree / PAGE_SIZE)) {
            pages[i].reference_count = 0;
            pages[i].next = NULL;
            if (prev != NULL) {
                prev->next = &pages[i];
            }
            prev = &pages[i];
        } else if (i >= (physfree / PAGE_SIZE) && i < ROUND_UP((physfree + end * sizeof(Page)), PAGE_SIZE) / PAGE_SIZE) {
            /* Mark memory used by page descriptor array as in use */
            pages[i].reference_count = 1;
            pages[i].next = NULL;
        } else {
            /* Mark rest of the memory pages as free */
            pages[i].reference_count = 0;
            pages[i].next = NULL;
            if (prev != NULL) {
                prev->next = &pages[i];
            }
            if (page_free_list == NULL) {
                page_free_list = &pages[i];
            }
            prev = &pages[i];
        }
    }

    index = end;
}

/* Allocate a page by returning first free page in the page free list */
Page *allocate_page() {
    if (page_free_list == NULL) {
        kprintf("No free pages for allocation\n");
        return NULL;
    }
    Page *free_page = page_free_list;
    page_free_list = page_free_list->next;
    free_page->reference_count = 1;

    return free_page;
}

/* Allocate pages by returning first free page in free list and moving head pointer num_pages ahead */
Page *allocate_pages(int num_pages) {
    if (page_free_list == NULL) {
        kprintf("No free pages for allocation\n");
        return NULL;
    }
    Page *free_page = page_free_list;
    while (num_pages-- > 0) {
        page_free_list->reference_count = 1;
        page_free_list = page_free_list->next;
    }

    return free_page;
}

/* Since the memory below physbase can now (after paging) be used as free, point page_free list to the 1st page */
void deallocate_initial_pages(uint64_t physbase) {
    pages[(physbase / PAGE_SIZE) - 1].next = page_free_list;
    page_free_list = &pages[1];
}
