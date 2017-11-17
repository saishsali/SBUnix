#include <sys/isr.h>

/* Get CR2 register value */
uint64_t get_cr2() {
    uint64_t cr2;
    __asm__ volatile(
        "movq %%cr2, %0;"
        : "=r"(cr2)
    );

    return cr2;
}

void page_fault_exception() {

}
