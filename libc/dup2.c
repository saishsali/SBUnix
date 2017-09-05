#include <sys/defs.h>

int dup2(int oldfd, int newfd) {

    __asm__ (
        "movq $33, %%rax;"
        "movq %0, %%rdi;"
        "movq %1, %%rsi;"
        "syscall;"
        :
        : "r" ((int64_t)oldfd), "r" ((int64_t)newfd)
    );

    return 1;
}
