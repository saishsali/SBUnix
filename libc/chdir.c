#include <sys/defs.h>

int chdir(char *path) {
    ssize_t output;
    __asm__ (
        "movq $7, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (output)
        : "r" (path)
        : "%rax", "%rdi"
    );

    return output;
}
