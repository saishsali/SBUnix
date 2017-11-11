#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>

task_struct *pcb1, *pcb2, *pcb0;

void _context_switch(task_struct *, task_struct *);

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

void schedule(int x) {
    kprintf("\n");
    if (x  == 1) {
        _context_switch(pcb1, pcb2);
    } else {
        _context_switch(pcb2, pcb1);
    }
}

void thread1() {
    while (1) {
        kprintf("Thread 1\n");
        // schedule();
        // _context_switch(pcb1, pcb2);
        schedule(1);
    }
}

void thread2() {
    while (1) {
        kprintf("Thread 2\n");
        // _context_switch(pcb2, pcb1);
        schedule(2);
    }
}

void create_process() {
    // char (*foo)(void) = thread1;
    // foo = thread1;

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
    // pcb0->pid = get_process_id();
    //     *((uint64_t *)&me->kstack[496]) = (uint64_t)thread1;
    // *((uint64_t *)&me->kstack[488]) = (uint64_t)me;


    // int i = 0;
    // while (i < 3) {
    //     i++;
    //     kprintf("Charu");
        _context_switch(pcb0, pcb1);
    // }

        // kprintf("aa gaya");
        // schedule(1);
        // kprintf("aa gaya 2");

        //         schedule(1);
        // kprintf("aa gaya 3");

        //         schedule(1);
        // kprintf("aa gaya 4");

        //         schedule(1);
        // kprintf("aa gaya 5");

        // schedule(1);
        // kprintf("aa gaya 6");

        // schedule(1);
        // kprintf("aa gaya 7");
        // while(1);




    // thread1();
}
