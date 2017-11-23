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

    } else {
        if ((current->file_descriptor[fd] != NULL) && (current->file_descriptor[fd]->permission != O_WRONLY)) {
            len_read = current->file_descriptor[fd]->cursor;
            len_end  = current->file_descriptor[fd]->node->last;
            if (length > (len_end - len_read))
                length = len_end - len_read;
            current->file_descriptor[fd]->cursor += length;
            memcpy((void *)buff, (void *)len_read, length);

            return length;
        }
    }

    return -1;
}

DIR* sys_opendir(char *path) {
    file_node *node;
    char *name;
    int i = 0, flag = 0;
    DIR* ret_dir;
    char directory_path[100];
    strcpy(directory_path, path);

    node = root_node;
    if (strcmp(directory_path, "/") != 0) {
        name = strtok(directory_path, "/");
        while (name != NULL) {
            flag = 0;
            if (strcmp(name, ".") == 0 ) {
                node = node->child[0];

            } else if (strcmp(name, "..") == 0) {
                node = node->child[1];

            } else {
                for (i = 2; i < node->last ; i++) {
                    if (strcmp(name, node->child[i]->name) == 0) {
                        node = node->child[i];
                        flag = 1;
                        break;
                    }
                }
                if(flag == 0) {
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

/* close the directory stream referred to by the argument dir */
int8_t sys_closedir(DIR *dir) {
    if (dir->cursor <= 1) {
        return -1;
    }

    if (dir->node->type == DIRECTORY) {
        dir->node->cursor = 0;
        dir->node = NULL;
        return 0;
    } else {
        return -1;
    }
}

int sys_getcwd(char *buf, size_t size) {
    if(size < strlen(current->current_dir))
        return -1;
    strcpy(buf, current->current_dir);
    return 1;
}

dentry* sys_readdir(DIR* dir) {
    if (dir->cursor > 1 && dir->node->last > 2 && dir->cursor < dir->node->last) {
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

void sys_close(int fd) {
    current->file_descriptor[fd] = NULL;
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
                free_user_memory((void *)virtual_address);
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
                free_user_memory((void *)virtual_address);
            }

            // Remove vma and add new vma with start as end address and end as vma->end
            flags = vma->flags;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, end_address, vma_end - end_address, flags, ANON);
            unmap = 1;
        } else if (start_address > vma->start && end_address < vma->end) {
            /*
                Case 3: When the specified address space overlaps partial VMA address space such that some
                part of lower and higher address space are not overlapped
            */
            for (virtual_address = start_address; virtual_address < end_address; virtual_address += PAGE_SIZE) {
                free_user_memory((void *)virtual_address);
            }

            // Remove vma and create 2 new VMA's
            flags = vma->flags;
            vma_start = vma->start;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, vma_start, start_address - vma_start, flags, ANON);
            add_vma(current, end_address, vma_end - end_address, flags, ANON);
            unmap = 1;
        } else if (start_address > vma->start && start_address < vma->end && end_address >= vma->end) {
            /*
                Case 4: When the specified address space overlaps partial VMA address space such that some
                part of lower address space is not overlapped
            */
            for (virtual_address = start_address; virtual_address < vma->end; virtual_address += PAGE_SIZE) {
                free_user_memory((void *)virtual_address);
            }

            // Remove vma and add new vma with start as vma start and end as start address
            flags = vma->flags;
            vma_start = vma->start;
            vma_end = vma->end;
            remove_vma(&vma, &current->mm, &prev);
            add_vma(current, vma_start, start_address - vma_start, flags, ANON);
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

/* Sys open to open files: http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html */
int8_t sys_open(char *path, uint8_t flags) {
    if (path == NULL) {
        return -1;
    }

    file_descriptor *fd = kmalloc(sizeof(file_descriptor));
    file_node *node = root_node;
    uint8_t flag = 0, i;
    char *name = strtok(path, "/");
    if (name == NULL) {
        return -1;
    }

    while (name != NULL) {
        if (strcmp(name, ".") == 0) {
            node = node->child[0];
        } else if (strcmp(name, "..") == 0) {
            node = node->child[1];
        } else {
            flag = 0;
            for (i = 2; i < node->last; i++) {
                if (strcmp(name, node->child[i]->name) == 0) {
                    node = node->child[i];
                    flag = 1;
                    break;
                }
            }

            if (flag == 0) {
                return -1;
            }
        }
        name = strtok(NULL, "/");
    }

    if (node->type == DIRECTORY) {
        return -1;
    }
    fd->node       = node;
    fd->permission = flags;
    fd->cursor     = node->cursor;

    for (i = 3; i < MAX_FD; ++i) {
        if (current->file_descriptor[i] == NULL) {
            current->file_descriptor[i] = fd;
            return i;
        }
    }

    return -1;
}

pid_t sys_fork() {
    task_struct *child_task = shallow_copy_task(current);
    // Setup child task stack
    setup_child_task_stack(current, child_task);
    add_process(child_task);

    return child_task->pid;
}

void sys_exit() {
    int parent_exist = 0;
    //mcheck if the parent exists for this child
    if(current->parent) {
        // remove child from its parent and also adjust the siblings list
        remove_child_from_parent(current);
        parent_exist = 1;
    }

    // check if children exists for this parent
    if(current->child_head) {
        //remove parent from its child and mark all child as zombies
        remove_parent_from_child(current);
    }

    // empty vma list
    empty_vma_list(current->mm->head, parent_exist);
    // empty page tables
    empty_page_tables(current->cr3);
    // empty file descriptor
    memset((void*)current->file_descriptor, 0, MAX_FD * 8);
    current->state = EXIT;

    // remove current task from schedule list
    remove_task_from_process_schedule_list(current);
    memset((void*)current->kstack, 0, 4096);

    sys_yield();

}

int sys_waitpid(int pid, int *status, int options) {
    // check if the parent has any child
    if(current->child_head == NULL)
        return -1;

    if(pid > 0) {
        current->wait_on_child_pid = pid;
    } else {
        current->wait_on_child_pid = 0;
    }

    current->state = WAITING;
    return current->wait_on_child_pid;
}


void syscall_handler(stack_registers * registers) {
    switch (registers->rax) {
        case 0:
            registers->rax = sys_read(registers->rdi, (char *)registers->rsi, registers->rdx);
            break;
        case 1:
            registers->rax = sys_write(registers->rdi, registers->rsi, registers->rdx);
            break;
        case 2:
            sys_yield();
            break;
        case 3:
            registers->rax = (uint64_t)sys_mmap((void *)registers->rdi, registers->rsi, registers->rdx);
            break;
        case 4:
            registers->rax = (uint64_t)sys_opendir((char *)registers->rdi);
            break;
        case 5:
            registers->rax = sys_getcwd((char *)registers->rdi, registers->rsi);
            break;
        case 6:
            registers->rax = sys_munmap((void *)registers->rdi, registers->rsi);
            break;
        case 7:
            registers->rax = sys_chdir((char *)registers->rdi);
            break;
        case 8:
            registers->rax = (uint64_t)sys_readdir((DIR *)registers->rdi);
            break;
        case 9:
            registers->rax = sys_closedir((DIR *)registers->rdi);
            break;
        case 10:
            registers->rax = sys_open((char *)registers->rdi, registers->rsi);
            break;
        case 11:
            sys_close(registers->rdi);
            break;
        case 12:
            registers->rax = (uint64_t)sys_fork();
            break;
        case 13:
            sys_exit(registers->rdi);
            break;
        case 14:
            sys_waitpid(registers->rdi, (int *)registers->rsi, registers->rdx);
            break;
    }
}
