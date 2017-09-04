#include <sys/defs.h>

ssize_t write(int fd, const void *buf, size_t count) {
    __asm__ (
        "movq $1, %%rax;"
        "movq %0, %%rdi;"
        "movq %1, %%rsi;"
        "movq %2, %%rdx;"
        "syscall;"
        :
        : "g" ((uint64_t)fd), "g" (buf), "g" (count)
    );

    return count;
}
