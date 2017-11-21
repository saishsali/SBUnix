#include <sys/defs.h>

// int chdir(const char *path) {
//     ssize_t output;
//     __asm__ (
//         "movq $80, %%rax;"
//         "movq %1, %%rdi;"
//         "syscall;"
//     	"movq %%rax, %0;"
//         : "=r" (output)
//         : "r" (path)
//         : "%rax", "%rdi"
//     );

//     return output;
// }

int chdir(char *path) {
    ssize_t output;
    __asm__ (
        "movq $7, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%r10, %0;"
        : "=r" (output)
        : "r" (path)
        : "%rax", "%rdi"
    );

    return output;
}