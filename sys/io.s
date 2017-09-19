.text

.global _x86_64_inb
_x86_64_inb:
	movq %rdi, %rdx
	inb %dx, %al
	retq
