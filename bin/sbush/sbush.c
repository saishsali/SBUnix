#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

char *read() {
    char *input = NULL;
    size_t len = 1024;
    ssize_t n;

    if ((n = getline(&input, &len, stdin)) == -1) {
        exit(EXIT_FAILURE);
    }

    return input;
}

int main(int argc, char* argv[]) {
    char *input;
    while (1) {
        printf("%s", "sbush> ");
        input = read();
        puts(input);
    }

    return 0;
}
