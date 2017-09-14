.text

.global _x86_64_asm_lidt
_x86_64_asm_lidt:

  lidt (%rdi)
