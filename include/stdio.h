#ifndef _STDIO_H
#define _STDIO_H

#include <sys/defs.h>

static const int EOF = -1;

int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);

char *gets(char *s);
ssize_t getline(char lineptr[], size_t *n);

#endif
