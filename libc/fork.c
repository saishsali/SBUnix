#include <sys/defs.h>

pid_t fork() {
    int64_t pid;

    __asm__ (
        "movq $12, %%rax;"
        "int $0x80;"
        "movq %%rax, %0"
        : "=r" (pid)
        :
        : "%rax"
    );

    return pid;
}
