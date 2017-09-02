#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

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
        if (command[0] != '#' && command[0] != '\n')
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

char **parse(char *input, int *is_bg) {
    int i = 0;
    char *token;
    char **tokens = malloc(sizeof(char*) * 1024);

    while ((token = strtok(input, " \t\r\n")) != NULL) {
        tokens[i++] = token;
        input = NULL;
    }
    tokens[i] = NULL;

    if (strcmp(tokens[i-1], "&") == 0) {
        *is_bg = 1;
        tokens[i-1] = NULL;
    }

    return tokens;
}

int change_directory(char **tokens) {
    if (chdir(tokens[1]) != 0) {
        puts("Error changing directory");
    }

    return 1;
}

int execute(char **tokens, int is_bg) {
    pid_t cpid, pid;
    int fd[2];

    if (is_bg == 1) {
        if (pipe(fd)) {
            fprintf(stderr, "Pipe failed\n");
            exit(EXIT_FAILURE);
        }
    }

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
            while(is_bg == 0 && (cpid = wait(NULL)) > 0);
            // Understand and implement
            // close(fd[0]);
            // close(fd[1]);
        }
    }

    return 1;
}

void lifetime(int argc, char* argv[]) {
    char **commands, **tokens, *command;
    int flag = 0, i = 0, is_bg = 0;

    if (argc >= 2) {
        commands = read_script(argv[1]);
        while (commands[i] != NULL) {
            tokens = parse(commands[i++], &is_bg);
            flag = execute(tokens, is_bg);
        }
    } else {
        do {
            printf("%s", "sbush> ");
            command = get_command();
            tokens = parse(command, &is_bg);
            flag = execute(tokens, is_bg);
            is_bg = 0;
        } while (flag);
    }
}

int main(int argc, char* argv[]) {
    lifetime(argc, argv);

    return 0;
}
