#include <sys/defs.h>
#include <sys/page_descriptor.h>
#include <sys/paging.h>
#include <sys/string.h>
#include <sys/process.h>
#include <sys/kprintf.h>

/* Allocate size bytes of page aligned memory memory in kernel space */
void *kmalloc(size_t size) {
    uint64_t start_address, virtual_address, num_pages;
    Page *p = NULL;

    num_pages = (ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE;
    p = allocate_pages(num_pages);
    start_address = page_to_virtual_address(p);
    virtual_address = start_address;

    while (num_pages--) {
        map_page(virtual_address, page_to_physical_address(p), RW_FLAG);
        memset((void *)virtual_address, 0, PAGE_SIZE);
        p = p->next;
        virtual_address = page_to_virtual_address(p);
    }

    return (void*) start_address;
}

/* Allocate size bytes of page aligned memory memory in user space */
void *kmalloc_user(size_t size) {
    static uint64_t virtual_address = VIRTUAL_BASE;
    uint64_t num_pages, start_address;
    Page *p = NULL;

    num_pages = (ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE;
    p = allocate_pages(num_pages);
    start_address = virtual_address;

    while (num_pages--) {
        map_page(virtual_address, page_to_physical_address(p), RW_FLAG);
        p = p->next;
        virtual_address += PAGE_SIZE;
    }

    return (void*) start_address;
}

/* Allocate size bytes of page aligned memory memory and map it to the specified virtual address in page tables */
void *kmalloc_map(size_t size, uint64_t virtual_address, uint16_t flags) {
    uint64_t num_pages, start_address;
    Page *p = NULL;

    num_pages = (ROUND_UP(size, PAGE_SIZE)) / PAGE_SIZE;
    p = allocate_pages(num_pages);
    start_address = virtual_address;

    while (num_pages--) {
        map_page(virtual_address, page_to_physical_address(p), flags);
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

/* Allocate new vma and add details */
vma_struct *allocate_vma(
    task_struct *task,
    uint64_t address,
    uint64_t size,
    uint64_t flags,
    uint64_t type
) {
    vma_struct *vma = kmalloc(sizeof(vma_struct));

    // Start and end addresses are 4K aligned always
    vma->start           = ROUND_DOWN(address, PAGE_SIZE);
    vma->end             = ROUND_UP(address + size, PAGE_SIZE);
    vma->next            = NULL;
    vma->flags           = flags;
    vma->type            = type;
    vma->mm              = task->mm;

    return vma;
}

/* Add new vma to the process list of vmas */
vma_struct *add_vma(
    task_struct *task,
    uint64_t address,
    uint64_t size,
    uint64_t flags,
    uint64_t type
) {
    vma_struct *new_vma = allocate_vma(task, address, size, flags, type);

    vma_struct *vma = task->mm->head;

    if (vma == NULL) {
        task->mm->head = new_vma;
        task->mm->tail = new_vma;
        return new_vma;
    }

    // Add new vma as a head node
    if (new_vma->end <= vma->start) {
        new_vma->next = vma;
        task->mm->head = new_vma;
        return new_vma;
    }

    // Add in the middle of the list
    while (vma->next != NULL) {
        if (new_vma->start >= vma->end && new_vma->end <= vma->next->start) {
            new_vma->next = vma->next;
            vma->next = new_vma;
            return new_vma;
        }
        vma = vma->next;
    }

    // Add in the end of the list
    vma->next = new_vma;
    task->mm->tail = new_vma;

    return new_vma;
}

/* Free memory allocated for a VMA */
void remove_vma(vma_struct **vma, mm_struct **mm, vma_struct **prev) {
    if (*vma == (*mm)->head) {
        // Remove head VMA
        (*mm)->head = (*vma)->next;
        free_kernel_memory(*vma);
        *vma = (*mm)->head;
    } else if (*vma == (*mm)->tail) {
        // Remove tail VMA
        (*prev)->next = (*vma)->next;
        free_kernel_memory(*vma);
        (*mm)->tail = *prev;
        *vma = NULL;
    } else {
        // Remove VMA from the middle
        (*prev)->next = (*vma)->next;
        free_kernel_memory(*vma);
        *vma = (*prev)->next;
    }
}

/* Free memory allocated for a list of VMAs */
void remove_vmas(mm_struct *mm) {
    vma_struct *curr_vma = mm->head;
    vma_struct *prev_vma = NULL;
    uint64_t virtual_address, physical_address;
    void *pte_entry;

    while (curr_vma != NULL) {
        virtual_address = curr_vma->start;
        while (virtual_address < curr_vma->end) {
            pte_entry = get_page_table_entry((void *)virtual_address);

            if(pte_entry != NULL) {
                physical_address = GET_ADDRESS(*(uint64_t *)pte_entry);

                if (physical_address) {
                    if (get_reference_count(physical_address) == 1) {
                        free_user_memory((uint64_t *)virtual_address);
                    } else {
                        decrement_reference_count(physical_address);
                    }
                }
            }
            virtual_address += PAGE_SIZE;
        }

        prev_vma = curr_vma;
        curr_vma = curr_vma->next;
        free_kernel_memory(prev_vma);
    }
    free_kernel_memory(mm);
}

/*
    - Get next available address in heap
    - First vma of heap is used to track the beginning of heap region and is not backed by a page
*/
uint64_t get_heap_address(task_struct *task, uint64_t size) {
     vma_struct *vma = task->mm->head;

    while (vma != NULL) {
        if (vma->type == HEAP) {
            break;
        }
        vma = vma->next;
    }

    while (vma->type != STACK) {
        if ((vma->next->start - vma->end) >= size) {
            return vma->end;
        }
        vma = vma->next;
    }

    return 0;
}
