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
void isr_common_stub();

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
    if (next == NULL) {
        return current;
    }

    process_list_tail->next = current;
    current->next = NULL;
    process_list_head = next;
    process_list_tail = current;

    return next;
}

/* Schedule next task, set TSS rsp and context switch */
void schedule() {
    task_struct *running_pcb = current;
    task_struct *next = strawman_scheduler();

    set_tss_rsp((void *)((uint64_t)next->kstack + 4096 - 8));
    set_cr3(next->cr3);

    current = next;
    _context_switch(running_pcb, next);
}

void user_thread1() {
    // while (1) {
    //     // write(1, "User thread: 1, ", 16);
    //     yield();
    // }
}

void user_thread2() {
    // while (1) {
    //     // write(1, "User thread: 2, ", 16);
    //     yield();
    // }
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
    pcb->u_rsp = 0;
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

    Elf64_Ehdr *elf_header = get_elf_header(filename);
    if (elf_header == NULL || is_elf_file(elf_header) == 0) {
        return NULL;
    }

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

    load_executable(pcb, filename, elf_header);
    add_process(pcb);

    return pcb;
}

/*
    - Copy VMA's
    - Copy File Descriptors
    - Mark all pages as read-only and COW in page tables (use bit 9 - unused by hardware in x86)
*/
task_struct *shallow_copy_task(task_struct *parent_task) {
    task_struct *child_task = create_new_task();
    file_descriptor *file_descriptor;
    vma_struct * parent_task_vma = parent_task->mm->head;
    // memcpy((void *)child_task->mm, (void *)parent_task->mm, sizeof(mm_struct));
    uint64_t physical_address, pte_flags, virtual_address;
    void *pte_entry;
    int i;

    // Copy process name
    strcpy(child_task->name, parent_task->name);

    // Copy file descriptors
    for (i = 0; i < MAX_FD; i++) {
        if (parent_task->file_descriptor[i] != NULL) {
            file_descriptor                 = kmalloc(sizeof(file_descriptor));
            file_descriptor->node           = parent_task->file_descriptor[i]->node;
            file_descriptor->cursor         = parent_task->file_descriptor[i]->cursor;
            file_descriptor->permission     = parent_task->file_descriptor[i]->permission;
            child_task->file_descriptor[i]  = file_descriptor;
        }
    }

    child_task->parent = parent_task;

    if (parent_task->child_head) {
        child_task->siblings = parent_task->child_head;
    }
    parent_task->child_head = child_task;

    while (parent_task_vma) {
        add_vma(
            child_task,
            parent_task_vma->start,
            parent_task_vma->end - parent_task_vma->start,
            parent_task_vma->flags,
            parent_task_vma->type
        );

        if (parent_task_vma->type == STACK) {
            virtual_address = parent_task_vma->end - 0x1000;

            while (virtual_address >= parent_task_vma->start) {
                set_cr3(parent_task->cr3);
                pte_entry = get_page_table_entry((void *)virtual_address);
                if (!(*(uint64_t *)pte_entry & PTE_P)) {
                    break; // Since stack is a contiguous memory
                }

                uint64_t *new_virtual_address = kmalloc(PAGE_SIZE);
                memcpy((void *)new_virtual_address, (void *)virtual_address, PAGE_SIZE);

                set_cr3(child_task->cr3);
                map_page(virtual_address, virtual_to_physical_address(new_virtual_address), RW_FLAG);

                virtual_address -= PAGE_SIZE;
            }

        } else {
            virtual_address = parent_task_vma->start;
            // Set copy on write bit and unset write bit in page table entries to allow sharing pages
            while (virtual_address < parent_task_vma->end) {
                set_cr3(parent_task->cr3);
                pte_entry = get_page_table_entry((void *)virtual_address);

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
        }
        parent_task_vma = parent_task_vma->next;
        set_cr3(parent_task->cr3);
    }

    return child_task;
}

/* Set CR3, Set TSS rsp and switch to ring 3 */
void switch_to_user_mode(task_struct *pcb) {
    set_cr3(pcb->cr3);
    set_tss_rsp((void *)((uint64_t)pcb->kstack + 4096 - 8));
    _switch_to_ring_3(pcb->entry, pcb->u_rsp);
}

/*
    - Setup child task stack as if it is called using fork() and set return value as 0 in RAX
    - Context switching to child task should return in isr.s to the instruction after the call to interrupt_handler
    - Populate the top of the stack with 5 entries that INT $0x80 pushes when parent calls fork()
    - Populate the top of the stack with 5 entries that INT $0x80 pushes when parent calls fork()
    - Set RAX = 0 as the return value of the child process
    - Copy all other register values from parent's stack to make sure that the state after returning to userspace
      remains unchanged
    - Leave empty entries in child stack to accomodate for pops in context_switch.s
    - Push child PCB as the last entry on child stack and point rsp to this entry
*/
void setup_child_task_stack(task_struct *parent_task, task_struct *child_task) {
    // User data segment (SS)
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 1]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 3]);

    // RSP
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 2]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 4]);

    // Eflags
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 3]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 5]);

    // Code segment
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 4]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 6]);

    // RIP
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 5]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 7]);

    // RAX = 0 (Return value to the child process)
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 8]) = 0UL;

    // RBX
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 9]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 11]);

    // RCX
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 10]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 12]);

    // RDX
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 11]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 13]);

    // RBP
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 12]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 14]);

    // RSI
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 13]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 15]);

    // RDI
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 14]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 16]);

    // R8
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 15]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 17]);

    // R9
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 16]) = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 18]);

    // Return to ISR popping logic
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 17]) = (uint64_t)isr_common_stub + 20;

    // Leave 13 entries from 18 to 30 to accomodate for pops in context_switch.s

    // Push PCB
    *((uint64_t *)&child_task->kstack[STACK_SIZE - 8 * 31]) = (uint64_t)child_task;

    // Set RSP
    child_task->rsp = (uint64_t)&child_task->kstack[STACK_SIZE - 8 * 31];

    // Set entry as RIP
    child_task->entry = *((uint64_t *)&parent_task->kstack[STACK_SIZE - 8 * 7]);
}

/* Setup user process stack with argument values */
void setup_user_process_stack(task_struct *task, char *argv[]) {
    uint64_t u_rsp = task->u_rsp, argv_address[32];
    uint16_t argc = 0, argv_length;
    int16_t i;
    uint64_t current_cr3 = get_cr3();
    char argv1[10][100];

    // Get number of arguments to be pushed on stack
    while (argv[argc] != NULL) {
        strcpy(argv1[argc], argv[argc]);
        argc++;
    }

    set_cr3(task->cr3);

    for (i = argc - 1; i >= 0; i--) {
        argv_length = strlen(argv1[i]) + 1;
        u_rsp = u_rsp - argv_length;
        memcpy((void *)u_rsp, argv1[i], argv_length);
        argv_address[i] = u_rsp;
    }

    u_rsp--;

    for (i = argc - 1; i >= 0; i--) {
        *(uint64_t *)u_rsp = argv_address[i];
        u_rsp--;
    }

    *(uint64_t *)u_rsp = argc;

    task->u_rsp = u_rsp;

    set_cr3(current_cr3);
}
