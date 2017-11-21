#include <sys/defs.h>

// ssize_t read(int fd, void *buf, size_t count) {
//     ssize_t num_bytes;

//     __asm__ (
//         "movq $0, %%rax;"
//         "movq %1, %%rdi;"
//         "movq %2, %%rsi;"
//         "movq %3, %%rdx;"
//         "syscall;"
//         "movq %%rax, %0;"
//         : "=r" (num_bytes)
//         : "r" ((int64_t)fd), "r" (buf), "r" (count)
//         : "%rax", "%rdi", "%rsi", "%rdx"
//     );

//     return num_bytes;
// }


ssize_t read(int fd, void *buf, size_t count) {
    ssize_t num_bytes;

    __asm__ (
        "movq $0, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "int $0x80;"
        "movq %%r10, %0;"
        : "=r" (num_bytes)
        : "r" ((int64_t)fd), "r" (buf), "r" (count)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );

    return num_bytes;
}