#ifndef _MMAN_H
#define _MMAN_H

#include <sys/defs.h>

void *mmap(void *start, size_t length, uint64_t flags);
int8_t munmap(void *addr, size_t len);

#endif
