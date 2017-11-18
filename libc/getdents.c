#include <sys/defs.h>
#include <sys/dirent.h>

int getdents(unsigned int fd, char *dirp, unsigned int count) {
    int64_t output;
    __asm__ (
        "movq $78, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (output)
        : "r" ((uint64_t)fd), "r" (dirp), "r" ((uint64_t)count)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );

    return output;
}
