#include <sys/defs.h>

int waitpid(int pid, int *status, int options) {
    int64_t cid;

    __asm__ (
        "movq $61, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (cid)
        : "r" ((int64_t)pid), "r" (status), "r" ((int64_t)options)
    );

    return cid;
}
