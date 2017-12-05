#include <sys/defs.h>

void shutdown() {
    __asm__ (
        "movq $17, %%rax;"
        "int $0x80;"
        :::
    );
}
