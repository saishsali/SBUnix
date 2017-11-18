#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/process.h>

#define NUM_SYSCALLS 3

int sys_write(int n, uint64_t str, int len) {
    kprintf("%s", str);
    // TODO return no of bytes written
    return 100;
}

int sys_read(int n) {
    return 0;
}

void sys_yield() {
    schedule();
}

void* syscall_tbl[NUM_SYSCALLS] = {
    sys_read,
    sys_write,
    sys_yield
};

void syscall_handler(uint64_t syscall_no) {
    void *func_ptr;

    if (syscall_no >= 0 && syscall_no < NUM_SYSCALLS) {
        func_ptr = syscall_tbl[syscall_no];
        __asm__ __volatile__(
            "callq %0;"
            :
            : "r" (func_ptr)
        );
    }
}

ssize_t write(int fd, const void *buf, size_t count) {
    ssize_t num_bytes;

    __asm__ __volatile__(
        "movq $1, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "movq %3, %%rdx;"
        "int $0x80;"
        "movq %%r10, %0;"
        : "=r" (num_bytes)
        : "r" ((int64_t)fd), "r" (buf), "r" (count)
        : "%rax", "%rdi", "%rsi", "%rdx"
    );

    return num_bytes;
}

void yield() {
    __asm__ __volatile__(
       "movq $2, %%rax;"
       "int $0x80;"
       : : :
   );
}
