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
        map_page(virtual_address, page_to_physical_address(p));
        memset((void *)virtual_address, 0, PAGE_SIZE);
        p = p->next;
        virtual_address = page_to_virtual_address(p);
    }

	return (void*) start_address;
}
