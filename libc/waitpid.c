#include <sys/defs.h>
#include <stdio.h>

int waitpid(int pid, int *status) {
    int64_t cid;

    __asm__ (
        "movq $14, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (cid)
        : "r" ((int64_t)pid), "r" (status)
        : "%rax", "%rdi", "%rsi"
    );

    return cid;
}
