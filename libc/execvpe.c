#include <sys/defs.h>

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    int64_t status;
    int slash = 0, i = 0;

    while (file[i] != '\0') {
        if (file[i++] == '/') {
            slash = 1;
            break;
        }
    }

    if (slash == 1) {
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
    } else {
        // Check environment variable and append it
        status = 0;
    }

    return status;
}
