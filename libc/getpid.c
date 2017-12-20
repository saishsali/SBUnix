#include <sys/defs.h>

pid_t getpid(void) {
    uint64_t pid;

    __asm__ (
        "movq $19, %%rax;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (pid)
        :
        : "%rax"
    );

    return (pid_t)pid;
}
