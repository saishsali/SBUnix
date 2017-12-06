#include <sys/defs.h>

int kill(pid_t pid, int sig) {
	int64_t result;
    __asm__ (
        "movq $16, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" ((int64_t)result)
        : "r" ((int64_t)pid)
        : "%rax", "%rdi"
    );
    return result;
}