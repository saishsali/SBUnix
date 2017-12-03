#include <stdio.h>
#include <string.h>
#include <unistd.h>

int puts(const char *s)
{
    for( ; *s != '\0'; ++s) if (putchar(*s) != *s) return EOF;
    return (putchar('\n') == '\n') ? 0 : EOF;
}
