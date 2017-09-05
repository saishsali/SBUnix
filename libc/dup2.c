#include <sys/defs.h>

int dup2(int oldfd, int newfd) {
	ssize_t output;
    __asm__ (
        "movq $33, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "syscall;"
    	"movq %%rax, %0;"
        : "=r" (output)
        : "r" ((int64_t)oldfd), "r" ((int64_t)newfd)
    );

    return output;
}
