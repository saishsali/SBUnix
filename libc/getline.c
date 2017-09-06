#include <sys/defs.h>
#include <unistd.h>
#include <stdio.h>

ssize_t getline(char lineptr[], size_t *n) {
	ssize_t output = read(0, lineptr, *n);
    return output;
}