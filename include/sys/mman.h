#ifndef _MMAN_H
#define _MMAN_H

#include <sys/defs.h>

void *sys_mmap(void *start, size_t length, uint64_t flags);
int8_t sys_munmap(void *addr, size_t len);

#endif