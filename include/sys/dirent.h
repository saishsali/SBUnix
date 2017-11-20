#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/defs.h>

#define stdin 0
#define stdout 1
#define stderr 2

#define O_RDONLY 0
#define O_WRONLY 1

typedef struct file_descriptor file_descriptor;
typedef struct file_node file_node;
typedef struct dir DIR;
typedef struct dirent dentry;
file_node* root_node;

struct dirent {
    uint64_t inode_no;
    char name[30];
    uint16_t d_reclen;

};

struct file_node {
    uint64_t first;
    uint64_t last;
    uint64_t cursor;
    struct file_node* child[20];
    char name[20];
    int type;
};

struct file_descriptor {
    uint64_t cursor;
    uint64_t permission;
    uint64_t inode_no;
    file_node* node;
};

struct dir {
    file_node* node;
    uint64_t cursor;
    dentry *dentry;
};

int getdents(unsigned int fd, char *dirp, unsigned int count);

#endif
