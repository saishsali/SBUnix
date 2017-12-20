#include <sys/dirent.h>

dentry* readdir(DIR *dir) {
    dentry* output;
    __asm__ (
        "movq $8, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (output)
        : "r" (dir)
        : "%rax", "%rdi"
    );

    return output;
}
