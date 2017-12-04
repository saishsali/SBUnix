#include <sys/defs.h>

void ps() {
    __asm__ (
        "movq $15, %%rax;"
        "int $0x80;"
        :::
    );
}
