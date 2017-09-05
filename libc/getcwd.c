#include <sys/defs.h>

char *getcwd(char *buf, size_t size) {
	char * output;
    __asm__ (
        "movq $79, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "syscall;"
    	"movq %%rax, %0;"
        : "=r" (output)
        : "r" (buf), "r" (size)
    );

    return output;
}
