#include <sys/dirent.h>

int8_t closedir(DIR *dir) {
    int64_t output;
    __asm__ (
        "movq $9, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (output)
        : "r" (dir)
        : "%rax", "%rdi"
    );

    return (int8_t)output;
}
