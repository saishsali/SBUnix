#ifndef _DIRENT_H
#define _DIRENT_H

#define NAME_MAX 255

struct dirent {
	char d_name[NAME_MAX+1];     /* inode number */
	unsigned short d_reclen;    /* length of this record */
	unsigned char  d_type;      /* type of file; not supported
                                 by all file system types */
};


typedef struct DIR DIR;


void readdir(const char *name);
int getdents(unsigned int fd, char *dirp, unsigned int count);

#endif
