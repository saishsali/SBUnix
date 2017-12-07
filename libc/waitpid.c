#include <sys/defs.h>
#include <stdio.h>

pid_t waitpid(int pid, int *status) {
    pid_t cid;

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
