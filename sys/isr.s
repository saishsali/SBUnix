.text

.global isr0
.global isr1

isr0:
    # Saves the processor state
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %r8
    pushq %r9

    call timer_interrupt

    # Restores the stack frame
    popq %r9
    popq %r8
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    # End-of-interrupt command
    movb $0x20, %al
    outb %al, $0x20

    iretq



isr1:
    
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %r8
    pushq %r9

    call keyboard_interrupt

    # Restores the stack frame
    popq %r9
    popq %r8
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    

    # End-of-interrupt command
    movb $0x20, %al
    outb %al, $0x20

    iretq
