#ifndef _MMAP_H
#define _MMAP_H

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

#endif
