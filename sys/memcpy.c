#include <sys/defs.h>

// Copy string from destination to source
void *memcpy(void *dest, void *src, size_t n) {
    char *source = (char *)src, *destination = (char *)dest;
   	int i;

    for (i = 0; i < n; i++)
        destination[i] = source[i];

    return destination;
}
