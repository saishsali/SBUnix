#include <sys/defs.h>

void exit(int status) {
    __asm__ (
        "movq $60, %%rax;"
        "movq %0, %%rdi;"
        "syscall;"
        :
        : "r" ((int64_t)status)
        :
    );
}
