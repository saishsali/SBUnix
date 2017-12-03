#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BUFSIZE 2048

extern char PS1[50], PATH[100];

char *getenv(const char *name) {
    if (strcmp(name, "PATH") == 0) {
        return PATH;
    } else if (strcmp(name, "PS1") == 0) {
        return PS1;
    }

    return NULL;
}
