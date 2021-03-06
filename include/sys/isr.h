#ifndef _ISR_H
#define _ISR_H

#include <sys/defs.h>

struct stack_registers {
    // Pushed by isr_common_stub
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    // Interrupt number and error code
    uint64_t interrupt_number;
    uint64_t error_code;

    // Registers pushed by IRETQ
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
}__attribute__((packed));

typedef struct stack_registers stack_registers;

#endif
