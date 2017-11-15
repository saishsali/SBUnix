.text

.global _jump_random

_jump_random:
     movq %rax, 0x23
     movq %ds, %rax
     movq %es, %rax 
     movq %fs, %rax 
     movq %gs, %rax
 
     movq %eax, %rsp
     pushq 0x23 
     pushq %rax 
     pushf
     pushq 0x1B
     pushq test
     iretq