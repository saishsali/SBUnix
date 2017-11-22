/* http://wiki.osdev.org/Page_fault */
#include <sys/isr.h>
#include <sys/page_fault.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>
#include <sys/paging.h>
#include <sys/page_descriptor.h>
#include <sys/string.h>

extern task_struct *current;

/* Get CR2 register value */
uint64_t get_cr2() {
    uint64_t cr2;
    __asm__ volatile(
        "movq %%cr2, %0;"
        : "=r"(cr2)
    );

    return cr2;
}

void page_fault_exception(stack_registers *registers) {
    uint64_t page_fault_address = get_cr2(), virtual_address, physical_address;
    void *pte_entry, *new_pte_entry;

    // When Present bit is not set, it is caused by a non-present page
    if ((registers->error_code & PF_P) == 0) {
        vma_struct *vma = current->mm->head;
        while (vma != NULL) {
            if (page_fault_address >= vma->start && page_fault_address < vma->end) {
                kmalloc_map(vma->end - vma->start, vma->start, RW_FLAG);
                break;
            }
            vma = vma->next;
        }

        if (vma == NULL) {
            kprintf("Segmentation Fault, Address: %x, Error: %x\n", page_fault_address, registers->error_code);
            while(1);
        }
    } else if (registers->error_code & 0x01) {
        // the fault is caused by a protection violation
        pte_entry = get_page_table_entry((void *)page_fault_address);

        // If write bit is not set and cow bit is set in the page table entry
        if (!(*(uint64_t *)pte_entry & PTE_W) && (*(uint64_t *)pte_entry & PTE_COW)) {
            physical_address = GET_ADDRESS(*(uint64_t *)pte_entry);
            if (get_reference_count(physical_address) > 1) {
                // If the page is being referenced by multiple processes
                virtual_address = (uint64_t)kmalloc_user(PAGE_SIZE);
                memcpy((void *)virtual_address, (void *)page_fault_address, PAGE_SIZE);
                new_pte_entry = get_page_table_entry((void *)virtual_address);
                physical_address = GET_ADDRESS(*(uint64_t *)new_pte_entry);
                *(uint64_t *)pte_entry = physical_address | RW_FLAG;
                free_user_memory((uint64_t *)virtual_address);
            } else {
                SET_WRITABLE((uint64_t *) pte_entry);
                UNSET_COPY_ON_WRITE((uint64_t *) pte_entry);
            }
        }
    }
}
