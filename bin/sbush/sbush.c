#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char *read_input() {
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

    token = strtok(input, " \t\r\n");

    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    tokens[i] = NULL;

    return tokens;
}

int change_directory(char **tokens) {
    if (chdir(tokens[1]) != 0) {
        puts("Error changing directory");
    }

    return 1;
}

int execute(char **tokens) {
    pid_t cpid, pid;

    if (strcmp(tokens[0], "cd") == 0) {
        return change_directory(tokens);
    } else if (strcmp(tokens[0], "exit") == 0) {
        return 0;
    } else {
        pid = fork();

        if (pid == 0) {
            if (execvp(tokens[0], tokens) == -1) {
                printf("-sbush: %s: command not found\n", tokens[0]);
            }
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            puts("Fork error");
        } else {
            while((cpid = wait(NULL)) > 0);
        }
    }

    return 1;
}

int main(int argc, char* argv[]) {
    char *input;
    char **tokens;
    int flag;

    do {
        printf("%s", "sbush> ");
        input = read_input();
        tokens = parse(input);
        flag = execute(tokens);
    } while (flag);

    return 0;
}
