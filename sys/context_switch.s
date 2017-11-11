.text

.global _context_switch
_context_switch:

    pushq %rdi
    movq %rsp, (%rdi)
    movq (%rsi), %rsp
    popq %rdi
    retq
