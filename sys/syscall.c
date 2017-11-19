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
        ret_dir->cursor = 2;
        ret_dir->node = node;
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

int sys_chdir(char *path) {
    char curr[100];
    strcpy(curr, current->current_dir);
    int i = strlen(curr) - 1;
    char directory_path[100];
    strcpy(directory_path, path);

    char *curr_name = strtok(directory_path, "/");

    while (curr_name != NULL) {

        if (strcmp(curr_name, ".") == 0) {

        } else if (strcmp(curr_name, "..") == 0) {
            if (strcmp(curr_name, ".") != 0) {
                for (i = strlen(curr) - 2; i >= 0; i--) {
                    if (curr[i] == '/') {
                        memcpy(curr, curr, i + 1);
                        curr[i+1] = '\0';
                        break;
                    }
                }
            }
            
        } else {
            kprintf("here");
            strcat(curr, curr_name);
            strcat(curr, "/");
        }

        // DIR* current_dir = sys_opendir(curr);
        // if (current_dir == NULL) {
        //     kprintf("\n%s: It is not a directory", curr);
        //     return -1;
        // } else {
        //     kprintf("Here part3");
        // }
        strcpy(current->current_dir, curr);

        curr_name = strtok(NULL, "/");

    }

    return 1;
}

void sys_yield() {
    schedule();
}

void *sys_mmap(void *start, size_t length, uint64_t flags) {
    if ((uint64_t)start < 0) {
        kprintf("Not a valid address\n");
        return NULL;
    } else if ((uint64_t)start == 0) {
        // Address not specified, use address after vma tail->end
        start = (uint64_t *)current->mm->tail->end;
    } else if (validate_address(current, (uint64_t)start, length) == 0) {
        kprintf("Address already in use\n");
        return NULL;
    }

    add_vma(current, (uint64_t)start, length, flags, ANON);

    return start;
}

void* syscall_tbl[NUM_SYSCALLS] = {
    sys_read,
    sys_write,
    sys_yield,
    sys_mmap,
    sys_opendir,
    sys_getcwd,
    sys_chdir
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
        "movq $4, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%r10, %0;"
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
        "movq %%r10, %0;"
        : "=r" ((int64_t)output)
        : "r" (buf), "r" (size)
        : "%rax", "%rdi", "%rsi"
    );

    return output;
}

int chdir(char *path) {
    ssize_t output;
    __asm__ (
        "movq $6, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%r10, %0;"
        : "=r" (output)
        : "r" (path)
        : "%rax", "%rdi"
    );

    return output;
}

