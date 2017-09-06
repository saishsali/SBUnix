#include <sys/defs.h>
#include <unistd.h>

int pipe(int pipefd[2]) {
	int64_t status;

    __asm__ (
        "movq $22, %%rax;"
        "movq %1, %%rdi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (status)
        : "r" (pipefd)
        : "%rax", "%rdi"
    );

    return status;
}
