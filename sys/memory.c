#include <sys/defs.h>
#include <sys/memory.h>
#include <sys/kprintf.h>
#include <sys/page_descriptor.h>

void *kmalloc(size_t size, int flags) {
	uint64_t num_pages;
	Page *p = NULL;

	if(size < PAGE_SIZE) {
		p = allocate_page();
	} else {
		num_pages = (ROUND_UP(size, PAGE_SIZE))/PAGE_SIZE;
		p = allocate_pages(num_pages);
	}
	return (void*) p;
}
