#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#include <sys/dirent.h>
#define MAX_PROCESS 10

#define STACK_START 0xF0000000
#define STACK_SIZE  0x2000

#define MAX_FD 10

typedef struct vm_area_struct vma_struct;
typedef struct mm_struct mm_struct;

struct file {
    uint64_t start;
    uint64_t offset;
    uint64_t size;
    uint64_t bss_size;
};

typedef struct file file;

typedef enum vma_types {TEXT, DATA, STACK, HEAP, ANON, NOTYPE} VMA_TYPE;

typedef enum vma_flag {RW, RX} VMA_FLAG;

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
    vma_struct *head;
    vma_struct *tail;
};

typedef enum { RUNNING, SLEEPING, ZOMBIE, READY } STATE;

struct PCB {
    uint64_t rsp;
    uint64_t u_rsp;
    char kstack[STACK_SIZE];
    uint64_t pid;
    STATE state;
    int exit_status;
    mm_struct *mm;
    struct PCB *next;
    uint64_t entry;
    uint64_t cr3;
    struct file_descriptor* file_descriptor[MAX_FD];
    char current_dir[100];
    char name[20];
    struct PCB *parent;
    struct PCB *siblings;
    struct PCB *child_head;
};

typedef struct PCB task_struct;

task_struct *process_list_head, *process_list_tail;

int process_ids[MAX_PROCESS];

void create_threads();
task_struct *create_user_process(char *);

task_struct *create_user_process();

void schedule();

task_struct *shallow_copy_task(task_struct *parent_task);
void switch_to_user_mode(task_struct *pcb);

#endif
