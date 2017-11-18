#ifndef _MEMORY_H
#define _MEMORY_H
#include <sys/process.h>

void *kmalloc(size_t size);
void *kmalloc_user(size_t size);
void *kmalloc_map(size_t size, uint64_t virtual_address);
int validate_address(task_struct *task, uint64_t address, uint64_t size);
vma_struct *add_vma(task_struct *task, uint64_t address, uint64_t size, uint64_t flags, uint64_t type, uint64_t file_descriptor);

#endif
