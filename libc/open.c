#include <sys/defs.h>
#include <unistd.h>

int open(const char *path, int flags) {
    int64_t output;
    __asm__ (
        "movq $10, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" ((int64_t)output)
        : "r" (path), "r" ((int64_t)flags)
        : "%rax", "%rdi", "%rsi"
    );

    return output;
}
