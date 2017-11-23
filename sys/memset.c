#include <sys/defs.h>
#include <sys/kprintf.h>

void *memset(void *str, int c, size_t n) {
    int i;
    char *s = (char *)str;

    for (i = 0; i < n; i++) {
        s[i] = c;
        // kprintf("%d s[i] i ", i);
    }

    // kprintf("All done");

    return s;
}
