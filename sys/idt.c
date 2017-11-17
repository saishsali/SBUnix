#include <sys/defs.h>
#include <sys/gdt.h>
#define SIZE 256

extern void isr0();
extern void isr14();
extern void isr32();
extern void isr33();

// Interrupt Descriptor Table
struct IDT {
    uint16_t offset_1; // offset 0 -> 15
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2; // offset 16 -> 31
    uint32_t offset_3; // offset 32 -> 63
    uint32_t zero;
}__attribute__((packed));

// IDT information
struct IDT_ptr {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

typedef struct IDT IDT;
IDT idt[SIZE];

typedef struct IDT_ptr IDT_ptr;
IDT_ptr idt_ptr;

void _x86_64_asm_lidt(IDT_ptr *idt_ptr);

// Set properties of IDT entry
void set_idt(int num, uint64_t base, uint8_t type_attr) {
    idt[num].type_attr = type_attr;
    idt[num].offset_1 = base & 0xFFFF;
    idt[num].offset_2 = (base >> 16) & 0xFFFF;
    idt[num].offset_3 = (base >> 32) & 0xFFFFFFFF;
}

void init_idt() {
    int i, type_attr;
    idt_ptr.limit = sizeof(IDT) * 256 - 1; // Size of the table - 1
    idt_ptr.base = (uint64_t)&idt; // Virtual address of the table

    // Load IDT
    _x86_64_asm_lidt(&idt_ptr);

    for (i = 0; i < SIZE; i++) {
        // Single kernel stack
        idt[i].ist = 0x00;

        // target ring (RPL) = 0, GDT = 0 and index  = 1
        idt[i].selector = 0x08;
    }

    // 32-bit Interrupt gate: 0x8E (P=1, DPL=00b, S=0, type=1110b => type_attr=1000_1110b=0x8E)
    type_attr = 0x8E;

    // CPU exceptions
    for (i = 0; i < 32; i++)
        set_idt(i, (uint64_t)isr0, type_attr);

    set_idt(14, (uint64_t)isr14, type_attr);

    // Timer Interrupt
    set_idt(0x20, (uint64_t)isr32, type_attr);

    // Keyboard Interrupt
    set_idt(0x21, (uint64_t)isr33, type_attr);
}
