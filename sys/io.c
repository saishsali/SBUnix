#include <sys/defs.h>

unsigned char _x86_64_inb(unsigned short int port);

unsigned char inb (unsigned short int port) {
    unsigned char ch = _x86_64_inb(port);
    return ch;
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile(
        "inl %w1, %0;"
        : "=a" (ret)
        : "Nd" (port)
    );

    return ret;
}

void outl (uint16_t port, uint32_t data) {
    __asm__ volatile(
        "outl %0, %w1;"
        :
        : "a"(data), "Nd"(port)
    );
}
