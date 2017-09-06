#include <sys/defs.h>
#include <unistd.h>

int close(int fd) {
    int64_t status;

    __asm__ (
        "movq $3, %%rax;"
        "movq %1, %%rdi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (status)
        : "r" ((int64_t)fd)
        : "%rax", "%rdi"
    );

    return status;
}
