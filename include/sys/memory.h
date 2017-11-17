#ifndef _MEMORY_H
#define _MEMORY_H

void *kmalloc(size_t size);
void *kmalloc_user(size_t size);
void *kmalloc_address(size_t size);

#endif
