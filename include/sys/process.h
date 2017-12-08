#ifndef _PROCESS_H
#define _PROCESS_H

#include <sys/defs.h>
#include <sys/dirent.h>
#define MAX_PROCESS 1000

#define STACK_START 0xF0000000
#define STACK_LIMIT 0x10000      // 64 KB (16 Pages each of 4096 bytes)

#define STACK_SIZE  0x800       // Kernel stack size 2048 bytes

#define MAX_FD 100

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

typedef enum { ZOMBIE, READY, WAITING, RUNNING, SLEEPING} STATE;

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
    uint16_t wait_on_child_pid;
    struct PCB *parent;
    struct PCB *siblings;
    struct PCB *child_head;
    uint32_t sleep_time;
};

typedef struct PCB task_struct;

task_struct *process_list_head, *process_list_tail;

int process_ids[MAX_PROCESS];

void create_threads();

task_struct *create_user_process(char *, char **, char **);

void schedule();

task_struct *shallow_copy_task(task_struct *parent_task);

void switch_to_user_mode(task_struct *pcb);

void setup_child_task_stack(task_struct *parent_task, task_struct *child_task);

void add_process(task_struct *pcb);

void setup_user_process_stack(task_struct *task, char *argv[], char *envp[]);

void remove_child_from_parent(task_struct *current);

void remove_parent_from_child(task_struct *parent_task);

void create_idle_process();

void remove_pcb(uint16_t pid);

void add_child_to_parent(task_struct* child_task, task_struct* parent_task);

void update_siblings(task_struct *old_task, task_struct *new_task);

void check_if_parent_waiting(task_struct *child);

#endif
