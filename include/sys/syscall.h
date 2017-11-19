#ifndef __SYSCALL_H
#define __SYSCALL_H
#include <sys/defs.h>
#include <sys/dirent.h>

void syscall_handler();
ssize_t write(int fd, const void *buf, size_t count);
void yield();
ssize_t read(int fd, void *buf, size_t count);
DIR* opendir(void *path);

// Delete this
void *sys_mmap(void *start, size_t length, uint64_t flags);

int getcwd(char *buf, size_t size);
int chdir(char *path);

#endif
