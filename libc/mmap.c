#include <sys/defs.h>

void *sys_mmap(void *start, size_t length, uint64_t flags) {
    void* result;
    __asm__ (
        "movq $3, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "int $0x80;"
        "movq %%rax, %0"
        : "=r" (result)
        : "r" (start), "r" (length), "r" ((int64_t)flags)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );

    return result;
}
