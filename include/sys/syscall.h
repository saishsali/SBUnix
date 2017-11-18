#ifndef __SYSCALL_H
#define __SYSCALL_H
#include <sys/defs.h>

void syscall_handler();
ssize_t write(int fd, const void *buf, size_t count);
void yield();

#endif
