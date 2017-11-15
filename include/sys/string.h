#ifndef _STRING_H
#define _STRING_H

#include <sys/defs.h>

void *memset(void *str, int c, size_t n);
void *memcpy(void *dest, void *src, size_t n);
int strcmp(const char *s1, const char *s2);

#endif
