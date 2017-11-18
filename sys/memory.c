#include <sys/defs.h>
#include <sys/page_descriptor.h>
#include <sys/paging.h>
#include <sys/string.h>
#include <sys/process.h>

void *kmalloc(size_t size) {
    uint64_t num_pages;
	Page *p = NULL;

    num_pages = (ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE;
	p = allocate_pages(num_pages);

	return (void*) page_to_virtual_address(p);
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
        p = p->next;
        virtual_address += PAGE_SIZE;
    }

    return (void*) start_address;
}

/* Validate if the address (upto size bytes) is not used by traversing vma list for a task */
int validate_address(task_struct *task, uint64_t address, uint64_t size) {
    if (address >= KERNBASE) {
        return 0;
    }
    vma_struct *vma = task->mm->head;

    while (vma != NULL) {
        if (address <= vma->start && (address + size) > vma->start) {
            return 0;
        } else if (address < vma->end && (address + size) >= vma->end) {
            return 0;
        } else if (address > vma->start && (address + size) < vma->end) {
            return 0;
        }
        vma = vma->next;
    }

    return 1;
}
