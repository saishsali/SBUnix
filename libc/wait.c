#include <sys/defs.h>

pid_t wait(int *status) {
    int64_t pid;

    __asm__ (
        "movq $22, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (pid)
        : "r" (status)
        : "%rax", "%rdi"
    );

    return pid;
}
