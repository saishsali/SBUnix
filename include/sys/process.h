#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#define MAX_PROCESS 10

#define STACK_START 0xF0000000
#define HEAP_START  0x08000000

typedef struct vm_area_struct vma_struct;
typedef struct mm_struct mm_struct;

struct vm_area_struct {
    mm_struct *mm;
    uint64_t start;
    uint64_t end;
    vma_struct *next;
};

struct mm_struct {
    vma_struct *mma;
};

typedef enum { RUNNING, SLEEPING, ZOMBIE } STATE;

struct PCB {
    uint64_t rsp;
    uint64_t u_rsp;
    char kstack[4096];
    uint64_t pid;
    STATE state;
    int exit_status;
    mm_struct *mm;
    struct PCB *next;
    uint64_t rip;
};

typedef struct PCB task_struct;

task_struct *process_list_head, *process_list_tail;

int process_ids[MAX_PROCESS];

void create_threads();

task_struct *create_user_process();

void switch_to_ring_3(task_struct *task);

void yield();

#endif
