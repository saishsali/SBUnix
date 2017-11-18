#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#define MAX_PROCESS 10

#define STACK_START 0xF0000000
#define HEAP_START  0x08000000

typedef struct vm_area_struct vma_struct;
typedef struct mm_struct mm_struct;

struct file {
    uint64_t start;
    uint64_t offset;
    uint64_t size;
    uint64_t bss_size;
};

typedef struct file file;

typedef enum vma_types {TEXT, DATA, STACK, HEAP, NOTYPE} VMA_TYPE;

struct vm_area_struct {
    mm_struct *mm;
    uint64_t start;
    uint64_t end;
    vma_struct *next;
    uint64_t flags;
    uint64_t type;
    file *file;
};

typedef struct vm_area_struct vma_struct;

struct mm_struct {
    vma_struct *head, *tail;
    uint64_t start_code, end_code, start_data, end_data;
};

typedef enum { RUNNING, SLEEPING, ZOMBIE, READY } STATE;

struct PCB {
    uint64_t rsp;
    uint64_t u_rsp;
    char kstack[4096];
    uint64_t pid;
    STATE state;
    int exit_status;
    mm_struct *mm;
    struct PCB *next;
    uint64_t entry;
};

typedef struct PCB task_struct;

task_struct *process_list_head, *process_list_tail;

int process_ids[MAX_PROCESS];

void create_threads();
task_struct *create_user_process(char *);

task_struct *create_user_process();

void schedule();

#endif
