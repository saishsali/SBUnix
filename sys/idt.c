#include <sys/defs.h>
#include <sys/gdt.h>
#define SIZE 256

struct IDT_entry {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
};

struct IDT_ptr {
    uint16_t limit;
    uint64_t base;
};

typedef struct IDT_entry IDT;
typedef struct IDT_ptr IDT_ptr;

IDT idt[SIZE];
IDT_ptr idt_ptr;

void set_idt(int irq, uint64_t base, uint8_t type_attr) {
    idt[irq].type_attr = type_attr;
    idt[irq].offset_1 = base & 0xFFFF;
    idt[irq].offset_2 = (base >> 16) & 0xFFFF;
    idt[irq].offset_3 = (base >> 32) & 0xFFFFFFFF;
}

void _x86_64_asm_lidt(IDT_ptr *idt_ptr);

void init_idt() {
    int i;
    idt_ptr.limit = sizeof(IDT) * 256 - 1;
    idt_ptr.base = (uint64_t)&idt;

    _x86_64_asm_lidt(&idt_ptr);

    for (i = 0; i < SIZE; i++) {
        idt[i].ist = 0;
        idt[i].selector = 8;
    }

    // set_idt(0, , 0x8E);
}
