.text

.global isr0
isr0:
    cli
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx

    call timer_interrupt

    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    sti
    iretq

.global isr1
isr1:
    cli
    call timer_interrupt
    sti
    iretq
