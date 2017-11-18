.text

.global _x86_64_asm_pic_remapping
_x86_64_asm_pic_remapping:

    # al is used for i/o port access
    movb $0x11, %al
    outb %al, $0x20  # Restart PIC1. 0x20 is command port for PIC1
    outb %al, $0xA0  # Restart PIC2. 0xA0 is command port for PIC2

    movb $0x20, %al
    outb %al, $0x21  # Start PIC1 at 32 (0x20)
    movb $0x28, %al
    outb %al, $0xA1  # Start PIC2 at 40 (0x28)

    movb $0x04, %al
    outb %al, $0x21  # Cascade PIC2 to PIC1
    movb $0x02, %al
    outb  %al, $0xA1

    # Disable interrupts
    # movb $0x0, %al
    movb $0x1, %al
    outb %al, $0x21
    outb %al, $0xA1

    sti
    retq
