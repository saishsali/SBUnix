#include <sys/defs.h>

int execvp(const char *file, char *const argv[]) {
    int64_t status;

    __asm__ (
        "movq $59, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (status)
        : "r" (file), "r" (argv)
    );

    return status;
}
