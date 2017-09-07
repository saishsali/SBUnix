#include <sys/defs.h>

int chdir(const char *path) {
    ssize_t output;
    __asm__ (
        "movq $80, %%rax;"
        "movq %1, %%rdi;"
        "syscall;"
    	"movq %%rax, %0;"
        : "=r" (output)
        : "r" (path)
        : "%rax", "%rdi"
    );

    return output;
}
