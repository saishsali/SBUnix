#include <sys/kprintf.h>
#include <sys/io.h>

void keyboard_interrupt() {
    unsigned char scan_code = inb(0x60);
    kprintf("%d", scan_code);
}
