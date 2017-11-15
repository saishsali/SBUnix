.text

.global _jump_random

_jump_random:
     cli
     movq %rax, 0x23
     movq %ds, %rax
     movq %es, %rax 
     movq %fs, %rax 
     movq %gs, %rax
 
     movq %rax, %rsp
     pushq 0x23
     pushq %rax 
     pushfq
     pushq 0x1B
     pushq %rdi
     sti
     iretq