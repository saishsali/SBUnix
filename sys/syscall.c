#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/process.h>
#include <sys/dirent.h>
#include <sys/memory.h>
#include <sys/keyboard.h>
#include <sys/memcpy.h>
#include <sys/string.h>
#include <sys/tarfs.h>
#include <sys/page_descriptor.h>
#include <sys/paging.h>
#include <sys/isr.h>

#define NUM_SYSCALLS 10

extern task_struct *current;

int sys_write(uint64_t fd, uint64_t str, int length) {
    if (fd == stdout || fd == stderr) {
        kprintf("%s", str);
    }
    return length;

}

int sys_read(uint64_t fd, char* buff, uint64_t length) {
    uint64_t len_read = 0;
    uint64_t len_end = 0;

    if (fd == stdin) {
        length = scanf(buff, length);
        return length;

    } else if ((current->file_descriptor[fd] != NULL) && (current->file_descriptor[fd]->permission != O_WRONLY)) {
        len_read = current->file_descriptor[fd]->cursor;
        len_end  = current->file_descriptor[fd]->node->last;
        if (length > (len_end - len_read))
            length = len_end - len_read;
        current->file_descriptor[fd]->cursor += length;
        memcpy((void *)buff, (void *)len_read, length);
        return length;

    }

    return -1;
}

DIR* sys_opendir(char *path) {
    file_node *node;
    char *name;
    int i = 0;
    DIR* ret_dir;

    node = root_node;
    if (strcmp(path, "/") != 0) {
        name = strtok(path,"/");
        while (name != NULL) {
            if (strcmp(name, ".") == 0 ) {
                node = node->child[0];

            } else if (strcmp(name, "..") == 0) {
                node = node->child[1];
            } else {
                for (i = 2; i < node->last ; i++) {
                    if (strcmp(name, node->child[i]->name) == 0) {
                        node = node->child[i];
                        break;
                    }
                }
                if (i == node->last) {
                    return (DIR *)NULL;
                }
            }
            name = strtok(NULL,"/");
        }
    }

    if (node->type == DIRECTORY) {
        ret_dir = (DIR *)kmalloc(sizeof(DIR));
        ret_dir->cursor = 2;
        ret_dir->node = node;
        return ret_dir;
    } else {
        return (DIR *)NULL;
    }

}

void sys_yield() {
    schedule();
}

void *sys_mmap(void *start, size_t length, uint64_t flags) {
    if ((uint64_t)start < 0) {
        // Not a valid address
        return NULL;
    } else if ((uint64_t)start == 0) {
        // Address not specified, use address after vma tail->end
        start = (uint64_t *)current->mm->tail->end;
    } else if (validate_address(current, (uint64_t)start, length) == 0) {
        // Address already in use
        return NULL;
    }

    add_vma(current, (uint64_t)start, length, flags, ANON);

    return start;
}

int8_t sys_munmap(void *addr, size_t len) {
    if (((uint64_t)addr & 0xFFF) != 0) {
        return -1;
    }

    uint64_t start_address = (uint64_t)addr;
    uint64_t end_address   = ROUND_UP(start_address + len, PAGE_SIZE);
    uint16_t flags;
    uint64_t vma_start, vma_end, virtual_address;

    vma_struct *vma = current->mm->head;
    if (vma == NULL) {
        return 0;
    }
    vma_struct *prev = NULL;

    while (vma != NULL) {
        if (end_address <= vma->start) {
            break;
        }

        if (vma->type != ANON) {
            prev = vma;
            vma  = vma->next;
            continue;
        }

        if (start_address <= vma->start && end_address >= vma->end) {
            for (virtual_address = vma->start; virtual_address < vma->end; virtual_address += PAGE_SIZE) {
                add_to_free_list((void *)virtual_address);
            }
            remove_vma(&vma, &current->mm, &prev);
        } else if (start_address <= vma->start && end_address > vma->start && end_address < vma->end) {
            for (virtual_address = vma->start; virtual_address < end_address; virtual_address += PAGE_SIZE) {
                add_to_free_list((void *)virtual_address);
            }

            // Remove vma and add new vma with start as end address and end as vma->end
            flags = vma->flags;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, end_address, vma_end, flags, ANON);

        } else if (start_address > vma->start && end_address < vma->end) {
            for (virtual_address = start_address; virtual_address < end_address; virtual_address += PAGE_SIZE) {
                add_to_free_list((void *)virtual_address);
            }

            // Remove vma and create 2 new VMA's
            flags = vma->flags;
            vma_start = vma->start;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, vma_start, start_address, flags, ANON);
            add_vma(current, end_address, vma_end, flags, ANON);
        } else if (start_address > vma->start && start_address < vma->end && end_address >= vma->end) {
            for (virtual_address = start_address; virtual_address < vma->end; virtual_address += PAGE_SIZE) {
                add_to_free_list((void *)virtual_address);
            }

            // Remove vma and add new vma with start as vma start and end as start address
            flags = vma->flags;
            vma_start = vma->start;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, vma_start, start_address, flags, ANON);
        } else {
            prev = vma;
            vma  = vma->next;
        }
    }

    return 1;
}

void* syscall_tbl[NUM_SYSCALLS] = {
    sys_read,
    sys_write,
    sys_yield,
    sys_mmap,
    sys_munmap,
    sys_opendir
};

void syscall_handler(stack_registers * registers) {
    void *func_ptr;

    uint64_t syscall_no = registers->rax;
    if (syscall_no >= 0 && syscall_no < NUM_SYSCALLS) {
        func_ptr = syscall_tbl[syscall_no];
        __asm__ __volatile__(
            "movq %0, %%rdi;"
            "movq %1, %%rsi;"
            "movq %2, %%rdx;"
            "callq %3;"
            :
            : "r" (registers->rdi), "r" (registers->rsi), "r" (registers->rdx), "r" (func_ptr)
            : "%rdi", "%rsi", "%rdx"
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


ssize_t read(int fd, void *buf, size_t count) {
    ssize_t num_bytes;

    __asm__ (
        "movq $0, %%rax;"
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


DIR* opendir(void *path) {
    DIR * ret_directory = NULL;
    __asm__ __volatile__(
        "movq $5, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%r10, %0;"
        : "=r" (ret_directory)
        : "r" (path)
        : "%rax", "%rdi"
    );
    return ret_directory;
}
