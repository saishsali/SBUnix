#ifndef __SYSCALL_H
#define __SYSCALL_H
#include <sys/defs.h>

void syscall_handler();
ssize_t write(int fd, const void *buf, size_t count);
void yield();
ssize_t read(int fd, void *buf, size_t count);

// Delete this
void *sys_mmap(void *start, size_t length, uint64_t flags);

#endif
