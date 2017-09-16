.text

.global _x86_64_asm_set_frequency
_x86_64_asm_set_frequency:
    cli
    movb $0x36, %al
    outb %al, $0x43

    mov $1193182, %ax
    outb %al, $0x40
    movb %ah, %al
    outb %al, $0x40
    sti

    retq
