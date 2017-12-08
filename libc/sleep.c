#include <sys/defs.h>

uint32_t sleep(uint64_t seconds) {
    uint64_t result;
    __asm__ (
        "movq $18, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" ((uint64_t)result)
        : "r" ((uint64_t)seconds)
        : "%rax", "%rdi"
    );
    return (uint32_t)result;
}
