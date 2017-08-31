#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *readl() {
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

void change_directory(char **tokens) {
    if (chdir(tokens[1]) != 0) {
        perror("sbush");
    }
}

void execute(char **tokens) {
    if (strcmp(tokens[1], "cd") == 0) {
        change_directory(tokens);
    }
}

int main(int argc, char* argv[]) {
    char *input;
    char **tokens;

    while (1) {
        printf("%s", "sbush> ");
        input = readl();
        tokens = parse(input);
        execute(tokens);
    }

    return 0;
}
