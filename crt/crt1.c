#include <stdlib.h>

void _start(void) {
  // call main() and exit() here
    __asm__ (
        "movq %rsp, %rbp;"
        "movq 0(%rbp), %rdi;"
        "lea 8(%rbp), %rsi;"
        "lea 8(%rbp, %rdi, 8), %rdx;"
        "call main;"

        "movq %rax, %rdi;"
        "movq $13, %rax;"
        "int $0x80;"
    );
}
