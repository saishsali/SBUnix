#include <sys/defs.h>
#include <sys/gdt.h>
#define SIZE 256

extern void isr0();
extern void isr1();

struct IDT_entry {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
}__attribute__((packed));

struct IDT_ptr {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

typedef struct IDT_entry IDT;
typedef struct IDT_ptr IDT_ptr;

IDT idt[SIZE];
IDT_ptr idt_ptr;

void set_idt(int num, uint64_t base, uint8_t type_attr) {
    idt[num].type_attr = type_attr;
    idt[num].offset_1 = base & 0xFFFF;
    idt[num].offset_2 = (base >> 16) & 0xFFFF;
    idt[num].offset_3 = (base >> 32) & 0xFFFFFFFF;
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

    for (i = 0; i < 31; i++)
        set_idt(i, (uint64_t)isr1, 0x8E);

    set_idt(0x20, (uint64_t)isr0, 0x8E);
}
