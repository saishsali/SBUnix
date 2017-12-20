#ifndef __SYSCALL_H
#define __SYSCALL_H
#include <sys/defs.h>
#include <sys/dirent.h>

void syscall_handler();

void sys_exit();

void sys_yield();

int sys_wait(int *);

void sys_shutdown();

#endif
