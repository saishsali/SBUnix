#include <stdlib.h>

void _start(void) {
  // call main() and exit() here
    __asm__ (
        "movq %rsp, %rbp;"
        "movq 0(%rbp), %rdi;"
        "lea 8(%rbp), %rsi;"
        "call main;"
        "movq $60, %rax;"
        "syscall;"
    );
}
