#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

char *builtins[] = {"cd", "export", "echo", "exit"};

int num_builtins() {
    return (sizeof(builtins)/sizeof(builtins[0]));
}

char *trim_quotes(char *val) {
    if(val[0] == '\"' || val[0] == '\'') {
        val++;
        val[strlen(val) - 1] = 0;
    }

    return val;
}

char *decode_dollar_variables(char *val) {
    char *text = val;
    char *return_value = (char*) malloc(1024*sizeof(char));
    int i=0;
    int j=0;
    while(*text) {
        if(*text == '$') {
            char *key = (char*) malloc(1024*sizeof(char));
            text++;
            i=0;
            while((*text>='a' && *text<='z') || (*text>='A' && *text<='Z')) {
                key[i] = *text;
                text++;
                i++;
            }
            char *key_value = getenv(key);
            strcat(return_value, key_value);
            j=j+strlen(key_value);
            free(key);
        }
        else {
            return_value[j] = *text;
            text++;
            j++;
        }
    }
    return_value[j]='\0';

    return return_value;
}

int set_env(char *line) {
    char *key = strtok(line, "=");
    char *value = strtok(NULL, "=");
    value = trim_quotes(value);
    value = decode_dollar_variables(value);
    setenv(key, value, 1);

    return 1;
}

int get_env(char *key) {
    if(key[0] == '$') {
        key++;
        printf("%s\n", getenv(key));
    }
    else {
        printf("%s\n", key);
    }

    return 1;
}

int change_directory(char **tokens) {
    if (chdir(tokens[1]) != 0) {
        puts("Error changing directory");
    }

    return 1;
}

int builtin_func(char **tokens) {
    if (strcmp(tokens[0], "export") == 0) {
        return set_env(tokens[1]);
    }
    else if (strcmp(tokens[0], "echo") == 0) {
        return get_env(tokens[1]);
    }
    else if (strcmp(tokens[0], "cd") == 0){
        return change_directory(tokens);
    }
    else if (strcmp(tokens[0], "exit") == 0) {
        return 0;
    }
}

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
        // Special case when export statement has extra spaces in it
        if (strcmp(token, "export") == 0) {
            tokens[i++] = strtok(NULL, "");
            break;
        }
        input = NULL;
    }
    tokens[i] = NULL;

    if (strcmp(tokens[i-1], "&") == 0) {
        *is_bg = 1;
        tokens[i-1] = NULL;
    }

    return tokens;
}

int check_pipes(char **tokens) {
    int i = 0;

    while (tokens[i] != NULL) {
        if(strcmp(tokens[i++], "|") == 0)
            return 1;
    }

    return 0;
}

int execute(char **tokens, int is_bg) {
    pid_t cpid, pid;
    int fd[2];
    int pipe_present = 0, builtin_i = 0;

    if (is_bg == 1) {
        if (pipe(fd)) {
            fprintf(stderr, "Pipe failed\n");
            exit(EXIT_FAILURE);
        }
    }

    for (builtin_i = 0; builtin_i < num_builtins(); builtin_i++) {
        if (strcmp(tokens[0], builtins[builtin_i]) == 0) {
            builtin_func(tokens);
            return 1;
        }
    }

    pipe_present = check_pipes(tokens);

    if (pipe_present == 1) {
        //execute_pipes();
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

    // Free pointers
    free(command);
}

int main(int argc, char* argv[]) {
    lifetime(argc, argv);

    return 0;
}
