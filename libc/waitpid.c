#include <sys/defs.h>
#include <stdio.h>

int waitpid(int pid, int *status, int options) {
    int64_t cid;

    __asm__ (
        "movq $61, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "movq $0, %%r10;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (cid)
        : "r" ((int64_t)pid), "r" (status), "r" ((int64_t)options)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10"
    );

    return cid;
}