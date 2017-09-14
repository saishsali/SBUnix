#include <sys/defs.h>
#include <sys/gdt.h>
#define SIZE 256

struct IDT {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2;
    uint16_t offset_3;
    uint32_t zero;
};

typedef struct IDT IDT;

IDT idt[SIZE];

void init_idt() {
    int i;

    for (i = 0; i < SIZE; i++) {
        idt[i].ist = 0;
        idt[i].selector = 8;
    }
}
