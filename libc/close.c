#include <sys/defs.h>
#include <unistd.h>

int8_t close(int fd) {
	int64_t result;
    __asm__ (
        "movq $11, %%rax;"
        "movq %0, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" ((int64_t)result)
        : "r" ((int64_t)fd)
        : "%rax", "%rdi"
    );
    return (int8_t)result;
}
