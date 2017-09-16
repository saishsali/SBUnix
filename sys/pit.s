.text

.global _x86_64_asm_set_frequency
_x86_64_asm_set_frequency:
    # Square wave generator mode, low byte/high byte access mode, 16-bit binary mode
    movb $0x36, %al
    outb %al, $0x43
    sti

    retq
