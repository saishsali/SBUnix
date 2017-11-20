/*
    - References: https://compas.cs.stonybrook.edu/course/cse506-f17/lectures/cse506-L5-scheduler.pdf
*/

#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>
#include <sys/gdt.h>
#include <sys/paging.h>
#include <sys/elf64.h>
#include <sys/syscall.h>
#include <sys/string.h>
#include <sys/page_descriptor.h>

void _context_switch(task_struct *, task_struct *);
void _switch_to_ring_3(uint64_t, uint64_t);

task_struct *current;

/* Get next free process id */
int get_process_id() {
    int i;
    for (i = 0; i < MAX_PROCESS; i++) {
        if (process_ids[i] == 0) {
            process_ids[i] = 1;
            return i;
        }
    }
    return -1;
}

/* Pick the first task from the list and put suspended task at the end of the list */
task_struct *strawman_scheduler() {
    current = process_list_head;
    task_struct *next = process_list_head->next;
    process_list_tail->next = current;
    current->next = NULL;
    process_list_head = next;
    process_list_tail = current;

    return next;
}

/* Schedule next task, set TSS rsp and context switch */
void schedule() {
    task_struct *next = strawman_scheduler();
    set_tss_rsp((void *)((uint64_t)next->kstack + 4096 - 8));
    _context_switch(current, next);
}

void user_thread1() {
    // while (1) {
        // char buf[1024];
        int u = write(1, "User thread: 1, ", 16);
        kprintf("\n ret value %d", u);
        // kprintf("\n write something -- ");
        // TODO - Return value comes wrong because of 51 line in isr.s
        // read(0, buf, 128);
        // kprintf("----%s-----", buf);
        // kprintf("");

    // kprintf("coming jere");
    // DIR* ret = opendir("/../../../rootfs/../../rootfs/bin");
    // if(ret == NULL) {
    //     kprintf("NULL");
    // } else {
    //     kprintf("ret node %s", ret->node->name);
    //     kprintf("It exists");
    // }

    // DIR* dir = opendir("/rootfs/bin");
    // if(dir == NULL) {
    //     kprintf("NULL directory");
    // } else {
    //     kprintf("exists,  %p", dir);
    // }


        while(1);
        // yield();
    // }
}

void user_thread2() {
    while (1) {
        int ret = write(0, "User thread: 2, ", 16);
        kprintf("Return value: %d\n", ret);
        yield();
    }
}

void kernel_thread1() {
    uint64_t *stack = kmalloc_user(4096);
    process_list_head->u_rsp = (uint64_t)stack + 4096 - 8;

    process_list_head->entry = (uint64_t)user_thread1;
    set_tss_rsp((void *)((uint64_t)process_list_head->rsp));
    _switch_to_ring_3(process_list_head->entry, process_list_head->u_rsp);
}

void kernel_thread2() {
    uint64_t *stack = kmalloc_user(4096);
    process_list_head->u_rsp = (uint64_t)stack + 4096 - 8;

    process_list_head->entry = (uint64_t)user_thread2;
    set_tss_rsp((void *)((uint64_t)process_list_head->rsp));
    _switch_to_ring_3(process_list_head->entry, process_list_head->u_rsp);
}

/* Add process to the end of the process list */
void add_process(task_struct *pcb) {
    if (process_list_head == NULL) {
        process_list_head = pcb;
        current = pcb;
    }
    if (process_list_tail == NULL) {
        process_list_tail = pcb;
    } else {
        process_list_tail->next = pcb;
        process_list_tail = pcb;
    }
}

/* Create thread by setting up stack and rsp */
task_struct *create_thread(void *thread) {
    task_struct *pcb = kmalloc(sizeof(task_struct));
    pcb->pid = get_process_id();
    *((uint64_t *)&pcb->kstack[511 * 8]) = (uint64_t)thread; // Push Return address
    /* Stack entries from 498 to 510 are reserved for 13 registers pushed/poped in context_switch.s */
    *((uint64_t *)&pcb->kstack[497 * 8]) = (uint64_t)pcb;    // Push PCB
    pcb->rsp = (uint64_t)&pcb->kstack[497 * 8];
    pcb->next = NULL;
    add_process(pcb);

    return pcb;
}

/* Switch 2 new threads (thread1, thread 2) and switch from current thread to thread 1 */
void create_threads() {
    task_struct *pcb0 = kmalloc(sizeof(task_struct));
    task_struct *pcb1 = create_thread(kernel_thread1);
    create_thread(kernel_thread2);
    _context_switch(pcb0, pcb1);
}

/* Create new task */
task_struct *create_new_task() {
    task_struct *pcb = kmalloc(sizeof(task_struct));

    pcb->mm = kmalloc(sizeof(mm_struct));
    pcb->mm->head = NULL;
    pcb->mm->tail = NULL;

    pcb->next = NULL;
    pcb->parent = NULL;
    pcb->child_head = NULL;
    pcb->siblings = NULL;
    memset(pcb->kstack, 0, STACK_SIZE);
    memset(pcb->file_descriptor, 0, MAX_FD * sizeof(file_descriptor));
    pcb->pid = get_process_id();
    pcb->state = READY;
    pcb->cr3 = (uint64_t)set_user_address_space();

    return pcb;
}

/* Create new user process */
task_struct *create_user_process(char *filename) {
    char curr_dir[30], new_curr_directory[1024];
    int i;
    task_struct *pcb = create_new_task();
    strcpy(pcb->name, filename);

    // Adding current working directory to pcb
    strcpy(curr_dir, "/rootfs/");

    for(i = strlen(filename) - 1; i >= 0; i--) {
        if(filename[i] == '/') {
            memcpy(new_curr_directory, filename, i + 1);
        }
    }
    strcat(curr_dir, new_curr_directory);
    strcpy(pcb->current_dir, curr_dir);
    pcb->rsp = (uint64_t)pcb->kstack + STACK_SIZE - 8;

    load_executable(pcb, filename);
    add_process(pcb);

    return pcb;
}

/*
    - Copy VMA's
    - Copy File Descriptors
    - Mark all pages as read-only and COW in page tables (use bit 9 - unused by hardware in x86)
*/
task_struct *copy_task_struct(task_struct *parent_task) {
    task_struct *child_task = create_new_task();
    file_descriptor *file_descriptor;
    vma_struct * parent_task_vma = parent_task->mm->head;
    memcpy((void *)child_task->mm, (void *)parent_task->mm, sizeof(mm_struct));
    uint64_t physical_address, pte_flags;
    int i;

    // Copy file descriptors
    for (i = 0; i < MAX_FD; i++) {
        if (parent_task->file_descriptor[i] != NULL) {
            file_descriptor = kmalloc(sizeof(file_descriptor));
            file_descriptor->node  = parent_task->file_descriptor[i]->node;
            file_descriptor->cursor  = parent_task->file_descriptor[i]->cursor;
            file_descriptor->permission  = parent_task->file_descriptor[i]->permission;
            child_task->file_descriptor[i] = file_descriptor;
        }
    }

    child_task->parent = parent_task;
    strcpy(child_task->name, parent_task->name);

    if (parent_task->child_head) {
        child_task->siblings = parent_task->child_head;
    }
    parent_task->child_head = child_task;

    while (parent_task_vma) {
        uint64_t virtual_address = parent_task_vma->start;

        // Set copy on write bit and unset write bit in page table entries to allow sharing pages
        while (virtual_address < parent_task_vma->end) {
            set_cr3(parent_task->cr3);
            void *pte_entry = get_page_table_entry((void *)virtual_address);

            if (*(uint64_t *) pte_entry & PTE_P) {
                // Set Read only and COW bit for parent process
                SET_READ_ONLY((uint64_t *) pte_entry);
                SET_COPY_ON_WRITE((uint64_t *) pte_entry);

                physical_address = GET_ADDRESS(*(uint64_t *) pte_entry);
                pte_flags = GET_FLAGS(*(uint64_t *) pte_entry);
                set_cr3(child_task->cr3);
                // Set Read only and COW bit for child process
                map_page(virtual_address, physical_address, pte_flags);
                increase_page_reference_count(physical_address);
            }
            virtual_address += PAGE_SIZE;
        }
        parent_task_vma = parent_task_vma->next;
        set_cr3(parent_task->cr3);
    }

    return child_task;
}
