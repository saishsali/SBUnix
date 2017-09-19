#include <sys/defs.h>

void *memset(void *str, int c, size_t n) {
    int i;
    char *s = (char *)str;

    for (i = 0; i < n; i++)
        s[i] = c;

    return s;
}
