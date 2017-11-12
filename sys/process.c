#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>

task_struct *pcb1, *pcb2, *pcb0;

void _context_switch(task_struct *, task_struct *);

int no_opt; // To avoid compiler optimization

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

void thread1() {
    while (no_opt >= 0) {
        no_opt = (no_opt + no_opt) / 2;
        kprintf("Thread A\n");
        _context_switch(pcb1, pcb2);
    }
}

void thread2() {
    while (no_opt >= 0) {
        no_opt = (no_opt + no_opt) / 2;
        kprintf("Thread B\n");
        _context_switch(pcb2, pcb1);

    }
}

void create_process() {
    pcb0 = kmalloc(sizeof(task_struct));

    pcb1 = kmalloc(sizeof(task_struct));
    pcb1->pid = get_process_id();
    *((uint64_t *)&pcb1->kstack[496]) = (uint64_t)thread1;
    *((uint64_t *)&pcb1->kstack[488]) = (uint64_t)pcb1;
    pcb1->rsp = (uint64_t)&pcb1->kstack[488];

    pcb2 = kmalloc(sizeof(task_struct));
    pcb2->pid = get_process_id();
    *((uint64_t *)&pcb2->kstack[496]) = (uint64_t)thread2;
    *((uint64_t *)&pcb2->kstack[488]) = (uint64_t)pcb2;
    pcb2->rsp = (uint64_t)&pcb2->kstack[488];

    _context_switch(pcb0, pcb1);
}
