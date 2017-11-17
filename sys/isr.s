.text

.macro INTERRUPT_HANDLER num
    .global isr\num
    isr\num:

        # Clear interrupt
        cli

        # Push all registers
        # pushq %rax
        pushq %rbx
        pushq %rcx
        pushq %rdx
        pushq %rbp
        pushq %rsi
        pushq %rdi
        pushq %r8
        pushq %r9

        call interrupt_handler\num

        # Restore all registers
        popq %r9
        popq %r8
        popq %rdi
        popq %rsi
        popq %rbp
        popq %rdx
        popq %rcx
        popq %rbx
        # popq %rax

        # End-of-interrupt command
        # movb $0x20, %al
        # outb %al, $0x20

        # Set interrupt
        sti
        iretq
.endm

INTERRUPT_HANDLER 0
INTERRUPT_HANDLER 32
INTERRUPT_HANDLER 33
INTERRUPT_HANDLER 128
