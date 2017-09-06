#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/defs.h>

struct dirent {
   unsigned long  d_ino;
   unsigned long  d_off;
   unsigned short d_reclen;
   char           d_name[];
};

typedef struct dirent dirent;
typedef struct DIR DIR;

void readdir(const char *name);
int getdents(unsigned int fd, char *dirp, unsigned int count);

#endif
