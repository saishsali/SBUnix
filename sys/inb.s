.text

.global _x86_64_asm_inb
_x86_64_asm_inb:
	movq %rdi, %rdx
	inb %dx, %al
	retq