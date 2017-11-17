#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/process.h>

#define NUM_SYSCALLS 3

int sys_write(int n, uint64_t str, int len) {
    kprintf("%s\n", str);
    return 100;
}

int sys_read(int n) {
    return 0;
}

void sys_yield() {
    yield();
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
