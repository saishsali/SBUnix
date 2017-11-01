#include <sys/process.h>
#include <sys/memory.h>
#include <sys/kprintf.h>

task_struct *me, *next;

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

void schedule() {
    __asm__ volatile(
        "pushq %rdi;"
    );

    __asm__ volatile(
        "movq %0, %%rsp;"
        :
        :"m"(me->rsp)
    );

    __asm__ volatile(
        "movq %%rsp, %0;"
        :"=r"(next->rsp)
    );

    __asm__ volatile(
        "popq %rdi;"
    );
}

void thread1() {
    while (1) {
        kprintf("Thread 1\n");
        schedule();
    }
}

void thread2() {
    while (1) {
        kprintf("Thread 2\n");
        schedule();
    }
}

void create_process() {
    me = kmalloc(sizeof(task_struct));
    me->state = RUNNING;
    me->pid = get_process_id();

    next = kmalloc(sizeof(task_struct));
    next->state = SLEEPING;
    next->pid = get_process_id();

    thread1();
}
