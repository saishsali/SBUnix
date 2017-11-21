#include <sys/defs.h>

int8_t sys_munmap(void *addr, size_t len) {
    int64_t result;
    __asm__ (
        "movq $6, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "int $0x80;"
        "movq %%rax, %0"
        : "=r" (result)
        : "r" (addr), "r" (len)
        : "%rax", "%rdi", "%rsi"
    );

    return (int8_t)result;
}
