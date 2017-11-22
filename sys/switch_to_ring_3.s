.text

.global _switch_to_ring_3
_switch_to_ring_3:

        cli
        movq %rsi, %rax

        # SS
        pushq $0x23

        # RSP
        pushq %rax

        # EFLAGS
        pushfq

        # Enable Interrupts after IRETQ is executed
        popq %rax
        orq $0x200, %rax
        pushq %rax

        # CS
        pushq $0x2b

        # RIP
        pushq %rdi
        iretq
