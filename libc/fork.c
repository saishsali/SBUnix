#include <sys/defs.h>

// pid_t fork() {
//     int64_t pid;

//     __asm__ (
//         "movq $57, %%rax;"
//         "syscall;"
//         "movq %%rax, %0"
//         : "=r" (pid)
//         :
//         : "%rax"
//     );

//     return pid;
// }


pid_t fork() {
    int64_t pid;

    __asm__ (
        "movq $12, %%rax;"
        "int $0x80;"
        "movq %%r10, %0"
        : "=r" (pid)
        :
        : "%rax"
    );

    return pid;
}