#include <sys/defs.h>

pid_t fork() {
    int64_t pid;

    __asm__ (
        "movq $56, %%rax;"
        "syscall;"
        "movq %%rax, %0"
        : "=r" (pid)
    );

    return pid;
}
