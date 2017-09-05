#ifndef _STRING_H
#define _STRING_H

#include <sys/defs.h>

size_t strlen(const char *s);
char *strtok(char *str, const char *delim);
char *strcat(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);

#endif
