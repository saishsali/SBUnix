#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char **read_script(char *filename) {
    FILE *fp;
    int i = 0;
    char *command;
    ssize_t n;
    size_t len = 0;
    char **commands = malloc(sizeof(char*) * 1024);

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((n = getline(&command, &len, fp)) != -1) {
        if (command[0] != '#')
            commands[i++] = command;
        command = NULL;
    }
    commands[i] = NULL;

    return commands;
}

char *get_command() {
    size_t len = 0;
    ssize_t n;
    char *command;

    if ((n = getline(&command, &len, stdin)) == -1) {
        exit(EXIT_FAILURE);
    }

    return command;
}

char **parse(char *input) {
    int i = 0;
    char *token;
    char **tokens = malloc(sizeof(char*) * 1024);

    while ((token = strtok(input, " \t\r\n")) != NULL) {
        tokens[i++] = token;
        input = NULL;
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

void lifetime(int argc, char* argv[]) {
    char **commands, **tokens, *command;
    int flag = 0, i = 0;

    if (argc >= 2) {
        commands = read_script(argv[1]);
        while (commands[i] != NULL) {
            tokens = parse(commands[i++]);
            flag = execute(tokens);
        }
    } else {
        do {
            printf("%s", "sbush> ");
            command = get_command();
            tokens = parse(command);
            flag = execute(tokens);
        } while (flag);
    }
}

int main(int argc, char* argv[]) {
    lifetime(argc, argv);

    return 0;
}
