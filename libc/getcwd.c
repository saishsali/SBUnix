#include <sys/defs.h>

int getcwd(char *buf, size_t size) {
    int64_t output;
    __asm__ __volatile__(
        "movq $5, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" ((int64_t)output)
        : "r" (buf), "r" (size)
        : "%rax", "%rdi", "%rsi"
    );

    return output;
}
