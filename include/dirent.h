#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/defs.h>


struct linux_dirent {
   unsigned long  d_ino;     /* Inode number */
   unsigned long  d_off;     /* Offset to next linux_dirent */
   unsigned short d_reclen;  /* Length of this linux_dirent */
   char           d_name[];  /* Filename (null-terminated) */
                             /* length is actually (d_reclen - 2 -
                                offsetof(struct linux_dirent, d_name)) */
};

typedef struct linux_dirent dirent;
typedef struct DIR DIR;

void readdir(const char *name);
int getdents(unsigned int fd, char *dirp, unsigned int count);

#endif
