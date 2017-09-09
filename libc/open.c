#include <sys/defs.h>
#include <unistd.h>

int open(const char *pathname, int flags, mode_t mode) {
    int64_t fd;

    __asm__ (
        "movq $2, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (fd)
        : "r" (pathname), "r" ((int64_t)flags), "r" (mode)
        : "%rax", "%rdi", "%rsi"
    );

    return fd;
}
