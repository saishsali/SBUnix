#include <sys/defs.h>
#include <sys/page_descriptor.h>
#include <sys/paging.h>
#include <sys/kprintf.h>

void *kmalloc(size_t size) {
	Page *p = NULL;

	if (size < PAGE_SIZE) {
		p = allocate_page();
	} else {
		p = allocate_pages((ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE);
	}

	return (void*) page_to_virtual_address(p);
}
