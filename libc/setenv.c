#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define BUFSIZE 2560

extern char PATH[100], PS1[50];

void setenv(const char *name, const char *value) {
    if (strcmp(name, "PATH") == 0) {
        strcpy(PATH, value);
    } else if (strcmp(name, "PS1") == 0) {
        strcpy(PS1, value);
    }
}
