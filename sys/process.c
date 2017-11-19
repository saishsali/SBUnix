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
        // write(1, "User thread: 1, ", 16);
        // kprintf("\n write something -- ");
        // TODO - Return value comes wrong because of 51 line in isr.s
        // read(0, buf, 128);
        // kprintf("----%s-----", buf);
        // kprintf("");

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

/* Create new user process */
task_struct *create_user_process(char *filename) {
    char curr_dir[30], new_filename[1024];
    int i;
    uint64_t current_cr3 = get_cr3();

    task_struct *pcb = kmalloc(sizeof(task_struct));
    pcb->pid = get_process_id();
    pcb->state = READY;
    pcb->cr3 = (uint64_t)set_user_address_space();
    set_cr3(pcb->cr3);

    mm_struct *mm = (mm_struct *)kmalloc(sizeof(mm_struct));
    mm->head = mm->tail = NULL;
    pcb->mm = mm;

    // Adding current working directory to pcb

    curr_dir[0] = '\0';
    strcat(curr_dir, "/rootfs/");

    for(i = strlen(filename) - 1; i >= 0; i--) {
        if(filename[i] == '/') {
            memcpy(new_filename, filename, i + 1);
        }
    }
    strcat(curr_dir, new_filename);

    kprintf("\n curr dorectory is %s ", curr_dir);

    strcpy(pcb->current_dir, curr_dir);

    pcb->rsp = (uint64_t)pcb->kstack + 4096 - 8;

    load_executable(pcb, filename);

    set_cr3(current_cr3);

    current = pcb;
    return pcb;
}
