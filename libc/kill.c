#include <sys/defs.h>

void kill(int pid) {
    __asm__ (
        "movq $13, %%rax;"
        "movq %0, %%rdi;"
        "int $0x80;"
        :
        : "r" ((int64_t)pid)
        : "%rax", "%rdi"
    );
}