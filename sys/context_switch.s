.text

.global _context_switch
_context_switch:

    cli
    pushq %rbx
    pushq %rdi
    movq %rsp, 0(%rdi)
    movq 0(%rsi), %rsp
    popq %rdi
    popq %rbx
    sti
    retq
