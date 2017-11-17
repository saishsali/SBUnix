/*
    - References: https://compas.cs.stonybrook.edu/course/cse506-f17/lectures/cse506-L5-scheduler.pdf
*/

#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>
#include <sys/gdt.h>

void _context_switch(task_struct *, task_struct *);
void _switch_to_ring_3(uint64_t, uint64_t);

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
    // set_tss_rsp((uint64_t *)current->rsp);
    set_tss_rsp((void *)((uint64_t)next->kstack + 4096 - 8));
    _context_switch(current, next);
}

void user_yield() {
    __asm__ __volatile__(
       "movq $2, %%rax;"
       "int $0x80;"
       : : :
   );
}

void process1() {

    while (1) {
        write(0, "Process 1 Ring 3", 16);
        user_yield();
    }
}

void process2() {
    while (1) {
        write(0, "Process 2 Ring 3", 16);
        user_yield();
    }
}

void thread1() {
    uint64_t *stack = kmalloc_user(4096);
    current->u_rsp = (uint64_t)stack + 4096 - 8;

    current->rip = (uint64_t)process1;
    set_tss_rsp((void *)((uint64_t)current->kstack + 4096 - 8));
    _switch_to_ring_3(current->rip, current->u_rsp);
}

void thread2() {
    uint64_t *stack = kmalloc_user(4096);
    next->u_rsp = (uint64_t)stack + 4096 - 8;

    next->rip = (uint64_t)process2;
    set_tss_rsp((void *)((uint64_t)next->kstack + 4096 - 8));
    _switch_to_ring_3(next->rip, next->u_rsp);
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
    /* Stack entries from 3984 to 4080 are reserved for 13 registers pushed/poped in context_switch.s */
    *((uint64_t *)&pcb->kstack[3976]) = (uint64_t)pcb;    // Push PCB
    pcb->rsp = (uint64_t)&pcb->kstack[3976];
    pcb->next = NULL;
    add_process(pcb);

    return pcb;
}

/* Switch 2 new threads (thread1, thread 2) and switch from current thread to thread 1 */
void create_threads() {
    task_struct *pcb0 = kmalloc(sizeof(task_struct));
    task_struct *pcb1 = create_thread(thread1);
    task_struct *pcb2 = create_thread(thread2);
    current = pcb1;
    next = pcb2;
    _context_switch(pcb0, pcb1);
}
