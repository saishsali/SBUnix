.text

.global _switch_to_ring_3
_switch_to_ring_3:

        cli
        # movq $0x23, %rax
        # movq %rax, %ds
        # movq %rax, %es
        # movq %rax, %fs
        # movq %rax, %gs

        movq %rsi, %rax
        pushq $0x23
        pushq %rax
        pushfq
        # popq %rax
        # orq $0x200, %rax
        # pushq %rax
        pushq $0x1b
        pushq %rdi
        iretq
