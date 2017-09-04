#include <stdlib.h>

void _start(void) {
  // call main() and exit() here
    __asm__ (
        "mov %rsp, %rbp;"
        "mov 0(%rbp), %rdi;"
        "mov 8(%rbp), %rsi;"
        "call main;"
        "mov $60, %rax;"
        "syscall;"
    );
}
