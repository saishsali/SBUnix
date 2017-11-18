#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/defs.h>

#define stdin 0
#define stdout 1
#define stderr 2

#define O_RDONLY 0
#define O_WRONLY 1

struct dirent {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_name[];
};

struct file_t {
    uint64_t first;
    uint64_t last;
    uint64_t current;
    struct file_t* child[20];
    char name[20];
    int type;
    uint64_t f_inode_no;
};
typedef struct file_t file_t;

struct file_descriptor {
    uint64_t cursor;
    uint64_t permission;
    uint64_t inode_no;
    file_t* node;
};
typedef struct file_descriptor file_descriptor;

typedef struct dirent dirent;
typedef struct DIR DIR;



int getdents(unsigned int fd, char *dirp, unsigned int count);



#endif
