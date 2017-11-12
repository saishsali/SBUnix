/*
    - References: https://compas.cs.stonybrook.edu/course/cse506-f17/lectures/cse506-L5-scheduler.pdf
*/

#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>

void _context_switch(task_struct *, task_struct *);

task_struct *current, *next;

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
void scheduler() {
    current = process_list_head;
    next = process_list_head->next;
    process_list_tail->next = current;
    current->next = NULL;
    process_list_head = next;
    process_list_tail = current;
}

void yield() {
    scheduler();
    _context_switch(current, next);
}

void thread1() {
    while (1) {
        kprintf("Thread A\n");
        yield();
    }
}

void thread2() {
    while (1) {
        kprintf("Thread B\n");
        yield();
    }
}

/* Add process to the end of the process list */
void add_process(task_struct *pcb) {
    if (process_list_head == NULL) {
        process_list_head = pcb;
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
    *((uint64_t *)&pcb->kstack[4088]) = (uint64_t)thread; // Push Return address
    *((uint64_t *)&pcb->kstack[4080]) = (uint64_t)pcb;    // Push PCB
    pcb->rsp = (uint64_t)&pcb->kstack[4072];              // 8 bytes (488 - 495) for rbx used by kprintf
    pcb->next = NULL;
    add_process(pcb);

    return pcb;
}

/* Switch 2 new threads (thread1, thread 2) and switch from current thread to thread 1 */
void create_threads() {
    task_struct *pcb0 = kmalloc(sizeof(task_struct));
    task_struct *pcb1 = create_thread(thread1);
    create_thread(thread2);
    _context_switch(pcb0, pcb1);
}
