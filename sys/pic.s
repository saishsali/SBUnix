.text

.global _x86_64_asm_pic_remapping
_x86_64_asm_pic_remapping:
    cli

    movb $0x11, %al
    outb %al, $0x20
    outb %al, $0xA0

    movb $0x20, %al
    outb %al, $0x21
    movb $0x28, %al
    outb %al, $0xA1

    movb $0x04, %al
    outb %al, $0x21
    movb $0x02, %al
    outb  %al, $0xA1

    movb $0x01, %al
    outb %al, $0x21
    outb %al, $0xA1

    sti

    retq
