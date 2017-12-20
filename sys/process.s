.text

.globl _flush_tlb
_flush_tlb:
    pushq %rax
    movq %cr3, %rax
    movq %rax, %cr3
    popq %rax
    retq
