#include <sys/defs.h>

pid_t getppid(void) {
    uint64_t pid;

    __asm__ (
        "movq $20, %%rax;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (pid)
        :
        : "%rax"
    );

    return (pid_t)pid;
}
