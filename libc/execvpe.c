#include <sys/defs.h>

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    int64_t status;

    __asm__ (
        "movq $59, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (status)
        : "r" (file), "r" (argv), "r" (envp)
    );

    return status;
}
