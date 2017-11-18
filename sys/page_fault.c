/* http://wiki.osdev.org/Page_fault */
#include <sys/isr.h>
#include <sys/page_fault.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>

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
    uint64_t page_fault_address = get_cr2();

    // When Present bit is not set, it is caused by a non-present page
    if ((registers->error_code & PF_P) == 0) {
        vma_struct *vma = current->mm->head;
        while (vma != NULL) {
            if (page_fault_address >= vma->start && page_fault_address < vma->end) {
                kmalloc_map(vma->end - vma->start, vma->start);
                break;
            }
            vma = vma->next;
        }

        if (vma == NULL) {
            kprintf("Segmentation Fault, Address: %x, Error: %x", page_fault_address, registers->error_code);
        }
    }
}
