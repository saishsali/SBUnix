#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read() {
    char *input = NULL;
    size_t len = 0;
    ssize_t n;

    if ((n = getline(&input, &len, stdin)) == -1) {
        exit(EXIT_FAILURE);
    }

    return input;
}

char **parse(char *input) {
    int i = 0;
    char *token;
    char **tokens = malloc(sizeof(char*) * 1024);

    token = strtok(input, " \t");

    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " \t");
    }
    tokens[i] = NULL;

    return tokens;
}

int main(int argc, char* argv[]) {
    char *input;
    char **tokens;

    while (1) {
        printf("%s", "sbush> ");
        input = read();
        tokens = parse(input);
        puts(tokens[1]);
    }

    return 0;
}
