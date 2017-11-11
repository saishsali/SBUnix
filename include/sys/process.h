#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#define MAX_PROCESS 10

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
    char kstack[4096];
    uint64_t pid;
    STATE state;
    int exit_status;
    mm_struct *mm;
};

typedef struct PCB task_struct;

int process_ids[MAX_PROCESS];

void create_process();

#endif
