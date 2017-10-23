#include <sys/defs.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
	uint64_t ret;
	 __asm__ (
        "movq $10, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "movq %4, %%rbx;"
        "movq %5, %%rcx;"
        "movq %6, %%r12;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (ret)
        : "r" (addr), "r" (length), "r" ((int64_t)prot), "r" ((int64_t)flags), "r" ((int64_t)fd), "r" ((int64_t)offset)
        : "%rax", "%rdi", "%rsi", "%rdx", "%rbx", "%rcx", "%r12"
    );

	return (void *)ret;
}