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

#define NUM_SYSCALLS 10

extern task_struct *current;

void _flush_tlb();

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
    char directory_path[100];
    strcpy(directory_path, path);

    node = root_node;
    if (strcmp(directory_path, "/") != 0) {
        name = strtok(directory_path, "/");
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
        ret_dir->node = node;
        ret_dir->dentry = (dentry*) kmalloc(sizeof(dentry));
        ret_dir->cursor = 2;
        return ret_dir;
    } else {
        return (DIR *)NULL;
    }

}

int sys_getcwd(char *buf, size_t size) {
    if(size < strlen(current->current_dir))
        return -1;
    strcpy(buf, current->current_dir);
    return 1;
}

dentry* sys_readdir(DIR* dir) {
    if(dir->cursor > 0 && dir->node->last > 2 && dir->cursor < dir->node->last) {
        strcpy(dir->dentry->name, dir->node->child[dir->cursor]->name);
        dir->cursor++;
        return dir->dentry;
    }
    return NULL;
}

int sys_chdir(char *path) {
    char curr[100];
    strcpy(curr, current->current_dir);
    char directory_path[100];
    strcpy(directory_path, path);
    int i;

    DIR* current_dir = sys_opendir(path);
    if (current_dir == NULL) {
        kprintf("\n%s: It is not a directory", path);
        return -1;
    }

    char *name = strtok(directory_path, "/");
    while (name != NULL) {

        if (strcmp(name, ".") == 0) {

        } else if (strcmp(name, "..") == 0) {

            if (strcmp(name, ".") != 0) {
                for (i = strlen(curr) - 2; i >= 0; i--) {
                    if (curr[i] == '/') {
                        memcpy(curr, curr, i + 1);
                        curr[i+1] = '\0';
                        break;
                    }
                }
            }

        } else {
            strcat(curr, name);
            strcat(curr, "/");
        }

        strcpy(current->current_dir, curr);

        name = strtok(NULL, "/");

    }

    return 1;
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

/* Sys munmap (https://linux.die.net/man/3/munmap) */
int8_t sys_munmap(void *addr, size_t len) {
    // Start address should be 4k aligned
    if (((uint64_t)addr & 0xFFF) != 0) {
        return -1;
    }

    uint64_t start_address = (uint64_t)addr;
    // 4k align end address
    uint64_t end_address   = ROUND_UP(start_address + len, PAGE_SIZE);
    uint16_t flags;
    uint64_t vma_start, vma_end, virtual_address;
    uint8_t unmap = 0;

    vma_struct *vma = current->mm->head;
    if (vma == NULL) {
        return 0;
    }
    vma_struct *prev = NULL;

    /*
        Traverse VMA list to find overlapping VMAs within start and end address
        and remove any mappings for those entire pages
    */
    while (vma != NULL) {
        // Since VMA's track memory in sorted order
        if (end_address <= vma->start) {
            break;
        }

        // Do not remove mappings for pages that are not anonymous
        if (vma->type != ANON) {
            prev = vma;
            vma  = vma->next;
            continue;
        }

        if (start_address <= vma->start && end_address >= vma->end) {
            // Case 1: When the specified address space overlaps entire VMA address space
            for (virtual_address = vma->start; virtual_address < vma->end; virtual_address += PAGE_SIZE) {
                add_to_free_list((void *)virtual_address);
            }

            // Remove vma
            remove_vma(&vma, &current->mm, &prev);
            unmap = 1;
        } else if (start_address <= vma->start && end_address > vma->start && end_address < vma->end) {
            /*
                Case 2: When the specified address space overlaps partial VMA address space such that
                some part of higher address space is not overlapped
            */
            for (virtual_address = vma->start; virtual_address < end_address; virtual_address += PAGE_SIZE) {
                add_to_free_list((void *)virtual_address);
            }

            // Remove vma and add new vma with start as end address and end as vma->end
            flags = vma->flags;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, end_address, vma_end, flags, ANON);
            unmap = 1;
        } else if (start_address > vma->start && end_address < vma->end) {
            /*
                Case 3: When the specified address space overlaps partial VMA address space such that some
                part of lower and higher address space are not overlapped
            */
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
            unmap = 1;
        } else if (start_address > vma->start && start_address < vma->end && end_address >= vma->end) {
            /*
                Case 4: When the specified address space overlaps partial VMA address space such that some
                part of lower address space is not overlapped
            */
            for (virtual_address = start_address; virtual_address < vma->end; virtual_address += PAGE_SIZE) {
                add_to_free_list((void *)virtual_address);
            }

            // Remove vma and add new vma with start as vma start and end as start address
            flags = vma->flags;
            vma_start = vma->start;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, vma_start, start_address, flags, ANON);
            unmap = 1;
        } else {
            prev = vma;
            vma  = vma->next;
        }
    }

    if (unmap == 1) {
        // If the mappings have been removed, flush TLB to inform changes made to the paging structure
        _flush_tlb();
    }

    return 1;
}

void* syscall_tbl[NUM_SYSCALLS] = {
    sys_read,
    sys_write,
    sys_yield,
    sys_mmap,
    sys_opendir,
    sys_getcwd,
    sys_munmap,
    sys_chdir,
    sys_readdir
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
        "movq %%rax, %0;"
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
        "movq %%rax, %0;"
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
        "movq $4, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (ret_directory)
        : "r" (path)
        : "%rax", "%rdi"
    );
    return ret_directory;
}

int getcwd(char *buf, size_t size) {
    int64_t output;
    __asm__ __volatile__(
        "movq $5, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" ((int64_t)output)
        : "r" (buf), "r" (size)
        : "%rax", "%rdi", "%rsi"
    );

    return output;
}

int chdir(char *path) {
    ssize_t output;
    __asm__ (
        "movq $7, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (output)
        : "r" (path)
        : "%rax", "%rdi"
    );

    return output;
}

dentry* readdir(DIR *dir) {
    dentry* output;
    __asm__ (
        "movq $8, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (output)
        : "r" (dir)
        : "%rax", "%rdi"
    );

    return output;
}

