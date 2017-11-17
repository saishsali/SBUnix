#ifndef _ISR_H
#define _ISR_H

#include <sys/defs.h>

struct stack_registers {
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t rdi;

    uint64_t interrupt_number;
    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
}__attribute__((packed));

typedef struct stack_registers stack_registers;

#endif
