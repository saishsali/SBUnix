#include <sys/defs.h>

void exit(int status) {
    __asm__ (
        "movq $13, %%rax;"
        "movq %0, %%rdi;"
        "int $0x80;"
        :
        : "r" ((int64_t)status)
        : "%rax", "%rdi"
    );
}
