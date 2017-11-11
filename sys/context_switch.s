.text

.global _context_switch
_context_switch:

    cli
    pushq %rdi
    movq %rsp, (%rdi)
    movq (%rsi), %rsp
    popq %rdi
    sti
    retq
