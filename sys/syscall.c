#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/kprintf.h>

// extern task_struct* current;

#define NUM_SYSCALLS 2

// These will get invoked in kernel mode. */
int sys_write(int n, uint64_t addr, int len)
{
    kprintf("I am here");
    return 1;
}

int sys_read(int n)
{
    return 0;
}

// Set up the system call table
void* syscall_tbl[NUM_SYSCALLS] = 
{
	sys_read,
    sys_write
};


void syscall_handler(void)
{
    uint64_t syscallNo;

    __asm__ __volatile__("movq %%rax, %0;" : "=r"(syscallNo));

    if (syscallNo >= 0 && syscallNo < NUM_SYSCALLS) {
        void *func_ptr;
        uint64_t ret;

        __asm__ __volatile__("pushq %rdx;");
        func_ptr = syscall_tbl[syscallNo];
        __asm__ __volatile__(
                "movq %%rax, %0;"
                // "popq %%rdx;"
                "callq *%%rax;"
                : "=a" (ret) : "r" (func_ptr)
                );
    }

    __asm__ __volatile__("iretq;");
}

