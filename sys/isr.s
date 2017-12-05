.text

.macro INTERRUPT_HANDLER num
    .globl isr\num
    isr\num:
        pushq $0
        pushq $\num
        jmp isr_common_stub
.ENDM

.macro INTERRUPT_HANDLER_EXCEPTION num
    .globl isr\num
    isr\num:
        pushq $\num
        jmp isr_common_stub
.ENDM


.globl isr_common_stub
isr_common_stub:
    # Clear interrupt
    cli

    # Push all registers
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rsp, %rdi
    call interrupt_handler

    # End-of-interrupt command
    movb $0x20, %al
    outb %al, $0x20

    # Restore all registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    addq $16 ,%rsp

    # Set interrupt
    sti
    iretq

INTERRUPT_HANDLER 0
INTERRUPT_HANDLER_EXCEPTION 14
INTERRUPT_HANDLER 32
INTERRUPT_HANDLER 33
INTERRUPT_HANDLER 128
