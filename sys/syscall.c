#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/process.h>
#include <sys/dirent.h>
#include <sys/memory.h>
#include <sys/memcpy.h>

#define NUM_SYSCALLS 3

extern task_struct *current;

int sys_write(uint64_t fd, uint64_t str, int length) {
    if (fd == stdout || fd == stderr) {
        kprintf("%s", str);

    } else if (fd > 2) {
         vma_struct *iter;

        if ((current->file_descriptor[fd]) == NULL) {
            return -1;

        } else if(((file_descriptor *)current->file_descriptor[fd])->permission == O_RDONLY ){
            kprintf("\n Not valid permissions");
            return -1;

        } else {
            uint64_t end = 0, cursor_pointer = 0;
            cursor_pointer = ((file_descriptor *)(current->file_descriptor[fd]))->cursor;

            //iterate till end of vma
            for (iter = current->mm->head; iter != NULL; iter = iter->next) {
                if(iter->vm_file_descriptor == fd){
                    end = iter->end;
                    break;
                }
            }

            // adjust the end of vma if the asked length is greater than the limit
            if (cursor_pointer + length > end) {
                kmalloc_map((cursor_pointer + length) - end, end);
                end = cursor_pointer + length;
            }

            memcpy((void *)cursor_pointer, (void *)str, length);

            ((file_descriptor *)(current->file_descriptor[fd]))->cursor += length;
        }
    }

    return length;

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
