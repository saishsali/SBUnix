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

extern task_struct *current, *process_list_head;

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
            if (len_read > len_end) {
                return 0;
            }
            if (length > (len_end - len_read))
                length = len_end - len_read;
            current->file_descriptor[fd]->cursor += length;
            memcpy((void *)buff, (void *)len_read, length);

            return length;
        }
    }

    return -1;
}

void add_slash_at_end(char *path) {
    int i = strlen(path) - 1;
    if (path[i] != '/') {
        path[++i] = '/';
        path[++i] = '\0';
    }
}

void remove_slash_from_start(char *path) {
    char copy_path[100];
    int i, len = strlen(path);
    for(i = 1; i < len; i++) {
        copy_path[i-1] = path[i];
    }
    copy_path[i-1] = '\0';

    strcpy(path, copy_path);
}

DIR* sys_opendir(char *dir_path) {
    file_node *node;
    char *name;
    int i = 0, flag = 0, c = 0, len;
    DIR* ret_dir;
    char directory_path[100], path[100];
    strcpy(path, dir_path);
    strcpy(directory_path, path);

    node = root_node;
    if (strcmp(path, "/") != 0) {

        add_slash_at_end(path);
        char temp[100];

        if(path[0] == '/') {
            i = 1;
        }
        len = strlen(path);
        for (; i < len; i++) {
            temp[c] = path[i];
            c++;
            if(path[i] == '/') {
                break;
            }
        }
        temp[c-1] = '\0';

        if(strcmp(current->current_dir, "/") == 0) {
            if(strcmp(temp, "rootfs") != 0 && strcmp(temp, ".") != 0 && strcmp(temp, "..") != 0) {
                return (DIR *)NULL;
            } else {
                flag = 1;
            }
        }

        if(strcmp(temp, "rootfs") == 0 || flag) {

            // It is a absolute path
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

        } else {
            // It is a relative path
            strcpy(directory_path, current->current_dir);

            if (path[0] == '/')
                remove_slash_from_start(path);

            strcat(directory_path, path);
            directory_path[strlen(current->current_dir) + strlen(path)] = '\0';

            DIR *new_directory = sys_opendir(directory_path);
            if(new_directory == NULL) {
                return (DIR *)NULL;
            }
            node = new_directory->node;
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

void add_slash_at_end_and_back(char s[100]) {
    char new_path[100];
    int len = strlen(s) - 1;
    if (s[0] == '/' && s[len] == '/') {
        strcpy(new_path, s);

    } else if (s[0] != '/') {
        new_path[0] = '/';
        strcat(new_path, s);
        if (s[len] != '/') {
            strcat(new_path, "/");
        }

    } else if(s[len] != '/') {
        strcpy(new_path, s);
        strcat(new_path, "/");
    }
    strcpy(s, new_path);
}

int sys_chdir(char *dir_path) {
    char curr[100];
    strcpy(curr, current->current_dir);
    char directory_path[100], path[100];
    strcpy(path, dir_path);
    strcpy(directory_path, path);
    int i = 0, c = 0, len = 0;

    if (strcmp(path, "/") == 0) {
        strcpy(current->current_dir, "/");
        return 0;
    }

    // if current working directory is / and path is not rootfs, then return
    if (strcmp(current->current_dir, "/") == 0) {
        if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0) {
            strcpy(current->current_dir, "/");
            return 0;
        }
    }

    DIR* current_dir = sys_opendir(path);


    if (current_dir == NULL) {
        kprintf("\n It is not a valid path\n");
        return -1;
    }

    if(path[0] == '/') {
        // absolute path
        len = strlen(path) - 1;
        if(path[len] != '/') {
            strcat(path, "/");
        }
        strcpy(current->current_dir, path);
        return 0;
    }

    add_slash_at_end(path);
    char temp[100];

    if (path[0] == '/') {
        i = 1;
    }
    len = strlen(path);
    for (; i < len; i++) {
        temp[c] = path[i];
        c++;
        if (path[i] == '/') {
            break;
        }
    }
    temp[c-1] = '\0';

    strcpy(directory_path, current->current_dir);

    if (path[0] == '/')
        remove_slash_from_start(path);

    strcat(directory_path, path);

    directory_path[strlen(current->current_dir) + strlen(path)] = '\0';

    DIR *new_directory = sys_opendir(directory_path);

    if (new_directory == NULL) {
        kprintf("\n%s: It is not a directory", path);
        return -1;
    }

    if (strcmp(temp, "rootfs") == 0) {

        char *name = strtok(directory_path, "/");
        while (name != NULL) {
            if (strcmp(name, ".") == 0) {
            } else if (strcmp(name, "..") == 0) {
                for (i = strlen(curr) - 2; i >= 0; i--) {
                    if (curr[i] == '/') {
                        memcpy(curr, curr, i + 1);
                        curr[i+1] = '\0';
                        break;
                    }
                }
            } else {
                strcat(curr, name);
                strcat(curr, "/");
            }

            strcpy(current->current_dir, curr);
            name = strtok(NULL, "/");
        }


    } else {
        // It is a relative path
        file_node *node = new_directory->node;
        char new_curr_directory[100];
        int index = 0;
        new_curr_directory[index++] = '/';
        while (node && strcmp(node->name, "/") != 0) {
            for (i = strlen(node->name) - 1; i>=0; i--) {
                new_curr_directory[index++] = node->name[i];
            }
            new_curr_directory[index++] = '/';
            // moving to parent node
            node = node->child[1];
        }

        new_curr_directory[index] = '\0';

        // Path reverse
        char more_curr_directory[100];
        int c = 0;
        for (i = index-1; i >= 0; i--) {
            more_curr_directory[c++] = new_curr_directory[i];
        }
        more_curr_directory[c] = '\0';

        memset(current->current_dir, 0, 100);
        strcpy(current->current_dir, more_curr_directory);
    }

    return 0;
}

void sys_close(int fd) {
    current->file_descriptor[fd] = NULL;
}

void sys_yield() {
    schedule();
}

void *sys_mmap(void *start, size_t length, uint64_t flags) {
    if ((uint64_t)start < 0) {              /* Not a valid address */
        return NULL;
    } else if ((uint64_t)start == 0) {      /* Address not specified, find available address in the heap */
        start = (uint64_t *)get_heap_address(current, length);
        if (start == 0) {
            return NULL;
        }
    } else if (validate_address(current, (uint64_t)start, length) == 0) { /* Address already in use */
        return NULL;
    }
    add_vma(current, (uint64_t)start, length, flags, HEAP);

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
        if (vma->type != HEAP) {
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
            add_vma(current, end_address, vma_end - end_address, flags, HEAP);
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
            add_vma(current, vma_start, start_address - vma_start, flags, HEAP);
            add_vma(current, end_address, vma_end - end_address, flags, HEAP);
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
            add_vma(current, vma_start, start_address - vma_start, flags, HEAP);
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

int remove_file_name_from_path(char *directory_path, char *file_name) {
    int i, index = 0;
    for (i = strlen(directory_path) - 2; i >= 0; i--) {
        if (directory_path[i] == '/') {
            directory_path[i+1] = '\0';
            break;
        }
        file_name[index++] = directory_path[i];
    }
    file_name[index] = '\0';
    return index;
}

/* Sys open to open files: http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html */
int8_t sys_open(char *file_path, uint8_t flags) {
    char directory_path[100], file_name[20], path[100];
    int i = 0, c = 0, flag = 0, index = 0;
    strcpy(path, file_path);
    if (path == NULL) {
        return -1;
    }

    //Check if it is just a file name
    int len = strlen(path);
    for(i = 0; i < len; i++) {
        if(path[i] == '/')
            flag = 1;
    }

    //Check if path is a directory
    DIR *current_dir = sys_opendir(path);
    if(current_dir != NULL) {
        kprintf("\n%s: Is a directory \n", path);
        return -1;
    }


    if(flag == 1) {
        strcpy(directory_path, path);
        index = remove_file_name_from_path(directory_path, file_name);

        current_dir = sys_opendir(directory_path);

        if(current_dir == NULL) {
            kprintf("\n%s: It is not a valid path", path);
            return -1;
        }
    }

    add_slash_at_end(path);
    char temp[100];

    if(path[0] == '/') {
        i = 1;
    }
    len = strlen(path);
    for (; i < len; i++) {
        temp[c] = path[i];
        c++;
        if(path[i] == '/') {
            break;
        }
    }
    temp[c-1] = '\0';

    if (strcmp(temp, "rootfs") == 0){
        strcpy(directory_path, path);
    } else {
        // maybe it was a relative path
        // Validate path
        add_slash_at_end(path);

        if(path[0] == '/') {
            i = 1;
        }

        strcpy(directory_path, current->current_dir);

        if (path[0] == '/')
            remove_slash_from_start(path);

        strcat(directory_path, path);
        directory_path[strlen(current->current_dir) + strlen(path)] = '\0';

        // remove file name
        index = remove_file_name_from_path(directory_path, file_name);

        DIR *new_directory = sys_opendir(directory_path);

        if (new_directory == NULL) {
            kprintf("\n%s: It is not a valid path", path);
            return -1;
        }

        char new_file_name[100];
        int c = 0;
        for(i = index-1; i >= 0; i--) {
            new_file_name[c++] = file_name[i];
        }
        new_file_name[c] = '\0';
        strcat(directory_path, new_file_name);

    }

    file_node *node = root_node;
    file_descriptor *fd = kmalloc(sizeof(file_descriptor));

    char *name = strtok(directory_path, "/");
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
    setup_child_task_stack(current, child_task);
    add_process(child_task);

    return child_task->pid;
}

/*
    - Create new process for the specified filename
    - Load executable in memory
    - Setup the stack of the new process to push specified arguments
    - Switch to the new mode for the new process
    - Consideration: Execvpe is called by a leaf node process (i.e. with no childs)
*/
int8_t sys_execvpe(char *file, char *argv[], char *envp[]) {
    // Child exit, mark the parent as ready that is waiting for it
    task_struct *task = create_user_process(file, argv, envp);

    if (task == NULL) {
        return -1;
    }

    // PID of current task is the PID of new task
    task->pid = current->pid;

    // Parent of current task is the parent of new task
    task->parent = current->parent;

    strcpy(task->current_dir, current->current_dir);

    // Copy file descriptors
    memcpy(
        (void *)task->file_descriptor,
        (void *)current->file_descriptor,
        sizeof(current->file_descriptor[0]) * MAX_FD
    );

    // Update sibling list of the current task to add new task in place of old task
    update_siblings(current, task);
    task->child_head = current->child_head;

    /* Update parent of all child processes */
    task_struct *process = task->child_head;
    while (process != NULL) {
        process->parent = task;
        process = process->siblings;
    }

    current->state = ZOMBIE;

    // empty vma list
    remove_vmas(current->mm);

    // empty page tables
    remove_page_tables(current->cr3);

    // empty file descriptor
    memset((void*)current->file_descriptor, 0, MAX_FD * 8);

    switch_to_user_mode(task);

    return -1;
}

void cleanup(task_struct *current) {
    if (current->parent) {
        check_if_parent_waiting(current);
    }

    // check if children exists for this parent
    if (current->child_head) {
        //remove parent from its child
        remove_parent_from_child(current);
    }

    // empty vma list
    remove_vmas(current->mm);

    // empty page tables
    remove_page_tables(current->cr3);

    // empty file descriptor
    memset((void*)current->file_descriptor, 0, MAX_FD * 8);
    current->state = ZOMBIE;
}

void sys_exit() {
    cleanup(current);
    sys_yield();
}

int sys_kill(pid_t pid) {
    task_struct * pcb = process_list_head;
    while (pcb != NULL) {
        if (pcb->pid == pid && pcb->state != ZOMBIE) {
            cleanup(pcb);
            /* If the process kills itself, switch to some other process */
            if (current == pcb) {
                sys_yield();
            }
            break;
        }
        pcb = pcb->next;
    }
    return 0;
}

int sys_waitpid(int pid, int *status, int options) {
    task_struct *child = current->child_head, *temp;
    /* check if the parent has any child */
    if (child == NULL || pid == 1 || pid == 2) {
        return -1;
    }

    /* Check if there are ZOMBIE childs and free memory */
    while (child != NULL) {
        if (child->state == ZOMBIE) {
            remove_child_from_parent(child);
            temp = child;
            child = child->siblings;
            remove_pcb(temp->pid);
        } else {
            child = child->siblings;
        }
    }

    if (current->child_head == NULL) {
        return -1;
    }

    if (pid > 0) {
        current->wait_on_child_pid = pid;
    } else {
        current->wait_on_child_pid = 0;
    }

    current->state = WAITING;

    sys_yield();

    /* Unlink child from parent and free pcb of the child */
    child = current->child_head;
    while (child != NULL) {
        if (child->pid == current->wait_on_child_pid) {
            remove_child_from_parent(child);
            remove_pcb(current->wait_on_child_pid);
            break;
        }
        child = child->siblings;
    }

    return current->wait_on_child_pid;
}

int sys_wait(int *status) {
    task_struct *child = current->child_head, *temp;
    /* check if the parent has any child */
    if (child == NULL) {
        return -1;
    }

    /* Check if there are ZOMBIE childs and free memory */
    while (child != NULL) {
        if (child->state == ZOMBIE) {
            remove_child_from_parent(child);
            temp = child;
            child = child->siblings;
            remove_pcb(temp->pid);
        } else {
            child = child->siblings;
        }
    }

    if (current->child_head == NULL) {
        return -1;
    }

    current->wait_on_child_pid = 0;
    current->state = WAITING;

    sys_yield();

    child = current->child_head;

    /* Unlink child from parent and free pcb of the child */
    while (child != NULL) {
        if (child->pid == current->wait_on_child_pid) {
            remove_child_from_parent(child);
            remove_pcb(current->wait_on_child_pid);
            break;
        }
        child = child->siblings;
    }

    return current->wait_on_child_pid;
}

void sys_ps() {
    task_struct * pcb = process_list_head;
    char process_states[5][10] = {
        "ZOMBIE",
        "READY",
        "WAITING",
        "RUNNING",
        "SLEEPING"
    };

    int i = 0;
    kprintf("\n | ----- |--------- | --------------- "
            "\n |  PID  | State    |  Process Name "
            "\n | ----- |--------- | --------------- ");


    while (pcb != NULL) {
        if (pcb->state != 0) {
            kprintf("\n |  %d    |  %s  |  %s  ", pcb->pid, process_states[pcb->state], pcb->name);
            i++;
        }
        pcb = pcb->next;
    }
}

void sys_shutdown() {
    task_struct *pcb = process_list_head, *temp;
    while (pcb != NULL) {
        if (pcb->state != ZOMBIE) {
            cleanup(pcb);
        }
        temp = pcb;
        pcb = pcb->next;
        remove_pcb(temp->pid);
    }

    clear_screen();
    kprintf("\n*****************************************************************************");
    kprintf("\n********************* SBUnix is now shutting down ***************************");
    kprintf("\n*****************************************************************************");
    while (1);
}

uint32_t sys_sleep(uint32_t seconds) {
    current->sleep_time = seconds;
    current->state = SLEEPING;
    sys_yield();
    return seconds;
}

pid_t sys_getpid() {
    return current->pid;
}

pid_t sys_getppid() {
    return current->parent->pid;
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
            registers->rax = sys_waitpid(registers->rdi, (int *)registers->rsi, registers->rdx);
            break;
        case 15:
            sys_ps();
            break;
        case 16:
            registers->rax = sys_kill(registers->rdi);
            break;
        case 17:
            sys_shutdown();
            break;
        case 18:
            registers->rax = sys_sleep(registers->rdi);
            break;
        case 19:
            registers->rax = sys_getpid();
            break;
        case 20:
            registers->rax = sys_getppid();
            break;
        case 21:
            registers->rax = sys_execvpe((char *)registers->rdi, (char **)registers->rsi, (char **)registers->rdx);
            break;
        case 22:
            registers->rax = sys_wait((int *)registers->rdi);
            break;
    }
}
