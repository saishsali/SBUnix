#include <sys/defs.h>
#include <sys/page_descriptor.h>
#include <sys/paging.h>
#include <sys/string.h>

void *kmalloc(size_t size) {
    uint64_t start_address, virtual_address, num_pages;
	Page *p = NULL;

    num_pages = (ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE;
	p = allocate_pages(num_pages);
    start_address = page_to_virtual_address(p);
    virtual_address = start_address;

    while (num_pages--) {
        // Pages are already mapped in page tables
        memset((void *)virtual_address, 0, PAGE_SIZE);
        p = p->next;
        virtual_address = page_to_virtual_address(p);
    }

	return (void*) start_address;
}

void *kmalloc_user(size_t size) {
    static uint64_t virtual_address = VIRTUAL_BASE;
    uint64_t num_pages, start_address;
    Page *p = NULL;

    num_pages = (ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE;
    p = allocate_pages(num_pages);
    start_address = virtual_address;

    while (num_pages--) {
        map_page(virtual_address, page_to_physical_address(p));
        memset((void *)virtual_address, 0, PAGE_SIZE);
        p = p->next;
        virtual_address += PAGE_SIZE;
    }

    return (void*) start_address;
}

void *kmalloc_map(size_t size, uint64_t virtual_address) {
    uint64_t num_pages, start_address;
    Page *p = NULL;

    num_pages = (ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE;
    p = allocate_pages(num_pages);
    start_address = virtual_address;

    while (num_pages--) {
        map_page(virtual_address, page_to_physical_address(p));
        memset((void *)virtual_address, 0, PAGE_SIZE);
        p = p->next;
        virtual_address += PAGE_SIZE;
    }

    return (void*) start_address;
}
