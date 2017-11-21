#include <sys/defs.h>

// char *getcwd(char *buf, size_t size) {
// 	char * output;
//     __asm__ (
//         "movq $79, %%rax;"
//         "movq %1, %%rdi;"
//         "movq %2, %%rsi;"
//         "syscall;"
//     	"movq %%rax, %0;"
//         : "=r" (output)
//         : "r" (buf), "r" (size)
//         : "%rax", "%rdi", "%rsi"
//     );

//     return output;
// }

int getcwd(char *buf, size_t size) {
    int64_t output;
    __asm__ __volatile__(
        "movq $5, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "int $0x80;"
        "movq %%r10, %0;"
        : "=r" ((int64_t)output)
        : "r" (buf), "r" (size)
        : "%rax", "%rdi", "%rsi"
    );

    return output;
}
