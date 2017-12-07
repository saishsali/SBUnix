#ifndef _FCNTL_H
#define _FCNTL_H
#include <sys/defs.h>

#define O_RDONLY 0
#define O_WRONLY 1

int open(const char *pathname, int flags);

#endif
