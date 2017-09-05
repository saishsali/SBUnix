#include <sys/defs.h>

int pipe(int pipefd[2]) {
	ssize_t output;
    __asm__ (
        "movq $22, %%rax;"
        "movq %1, %%rdi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (output)
        : "r" (pipefd)
    );

    return output;
}
