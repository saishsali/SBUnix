#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/kprintf.h>

#define NUM_SYSCALLS 2

int sys_write(int n, uint64_t str, int len) {
    kprintf("Value -  %s\n", str);
    return 100;
}

int sys_read(int n) {
    return 0;
}

void* syscall_tbl[NUM_SYSCALLS] = {
	sys_read,
    sys_write
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