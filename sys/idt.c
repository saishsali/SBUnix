#include <sys/defs.h>
#include <sys/gdt.h>

struct IDT {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2;
    uint16_t offset_3;
    uint32_t zero;
};
