#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

char *trim_quotes(char *str) {
    if(str[0] == '\"' || str[0] == '\'') {
        str++;
        str[strlen(str) - 1] = 0;
    }

    return str;
}

int is_alphabet(char c) {
    if ((c >='a' && c <='z' ) || ( c >= 'A' && c <= 'Z'))
        return 1;

    return -1;
}

char *decode_environment_variable(char *var) {
    char *decoded_var = (char*) malloc(1024 * sizeof(char));
    int i = 0, j = 0;

    while(*var) {
        if(*var == '$' && (is_alphabet(*var + 1) == 1)) {
            char *name = (char*) malloc(1024 * sizeof(char));
            var++;
            i = 0;
            while(is_alphabet(*var) == 1) {
                name[i++] = *var;
                var++;
            }
            char *value = getenv(name);
            strcat(decoded_var, value);
            j = j + strlen(value);
        } else {
            decoded_var[j++] = *var;
            var++;
        }
    }
    decoded_var[j] = '\0';

    return decoded_var;
}

int set_environment_variable(char *line) {
    char *name = strtok(line, "=");
    char *value = strtok(NULL, "=");
    value = decode_environment_variable(trim_quotes(value));
    setenv(name, value, 1);

    return 1;
}

int get_environment_variable(char *name) {
    if(name[0] == '$') {
        name++;
        printf("%s\n", getenv(name));
    } else {
        printf("%s\n", name);
    }

    return 1;
}

int change_directory(char **tokens) {
    if (chdir(tokens[1]) != 0) {
        printf("%s\n", "Error changing directory");
    }

    return 1;
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

char **parse(char *command, int *is_bg) {
    int i = 0;
    char *token;
    char **tokens = malloc(sizeof(char*) * 1024);

    while ((token = strtok(command, " \t\r\n")) != NULL) {
        tokens[i++] = token;
        // Special case when export statement has extra spaces in it
        if (strcmp(token, "export") == 0) {
            tokens[i++] = strtok(NULL, "");
            break;
        }
        command = NULL;
    }
    tokens[i] = NULL;

    if (i > 0 && strcmp(tokens[i-1], "&") == 0) {
        *is_bg = 1;
        tokens[i-1] = NULL;
    }

    return tokens;
}

int check_pipes(char **tokens) {
    int i = 0;

    while (tokens[i] != NULL) {
        if (strcmp(tokens[i++], "|") == 0)
            return 1;
    }

    return -1;
}

int builtin_command(char **tokens) {
    if (strcmp(tokens[0], "export") == 0) {
        return set_environment_variable(tokens[1]);
    } else if (strcmp(tokens[0], "echo") == 0) {
        return get_environment_variable(tokens[1]);
    } else if (strcmp(tokens[0], "cd") == 0) {
        return change_directory(tokens);
    } else if (strcmp(tokens[0], "exit") == 0) {
        return 0;
    }

    return -1;
}

void execute_pipes(char **tokens) {
    int num_cmnds = 0, i = 0, j = 0, iterate = 0, pipe1[2], pipe2[2];
    pid_t pid;
    char **command = (char**) malloc(1024 * sizeof(char*));

    while (tokens[i] != NULL) {
        if(strcmp(tokens[i++], "|") == 0)
            num_cmnds++;
    }

    num_cmnds++;
    i = 0;
    while (tokens[i] != NULL) {
        j = 0;
        while(tokens[i] != NULL && strcmp(tokens[i], "|") != 0) {
            command[j++] = tokens[i++];
        }
        command[j] = NULL;

        // pipe1 or pipe2 depends on which pipe was active previously
        if (iterate & 1) {
            if (pipe(pipe1) == -1)
                printf("%s\n", "Pipe failed");
        }
        else {
            if (pipe(pipe2) == -1)
                printf("%s\n", "Pipe failed");
        }

        //pipe1 is for odd command and pipe2 is for even command
        pid = fork();

        if(pid == 0) {
            if(iterate == 0) {
                dup2(pipe2[1], 1);
            }
            else {
                if(iterate & 1) { //odd
                    dup2(pipe2[0], 0);
                    if (iterate != num_cmnds - 1)
                        dup2(pipe1[1], 1);
                } else {
                    dup2(pipe1[0], 0);
                    if(iterate != num_cmnds - 1)
                        dup2(pipe2[1], 1);
                }
            }
            if (execvp(command[0], command) == -1) {
                printf("-sbush: %s: command not found\n", command[0]);
            }
        }
        else {
            if (iterate == 0){
                close(pipe2[1]);
            } else {
                if (iterate & 1) {
                    close(pipe2[0]);
                    if (iterate != num_cmnds - 1)
                        close(pipe1[1]);
                } else {
                    close(pipe1[0]);
                    if (iterate != num_cmnds - 1)
                        close(pipe2[1]);
                }
            }
            waitpid(pid,NULL,0);
        }

        if (tokens[i] == NULL)
            break;
        i++;
        iterate++;
    }
}

int execute(char **tokens, int is_bg) {
    pid_t cpid, pid;
    int builtin;

    if (tokens[0] == NULL)
        return 1;

    if ((builtin = builtin_command(tokens)) != -1)
        return builtin;

    if (check_pipes(tokens) == 1) {
        execute_pipes(tokens);
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
            if (is_bg == 0)
                while((cpid = waitpid(pid, NULL, 0)) > 0);
        }
    }

    return 1;
}

void lifetime(int argc, char* argv[]) {
    char *command, **commands, **tokens;
    int flag = 0, i = 0, is_bg = 0;

    if (argc >= 2) {
        commands = read_script(argv[1]);
        while (commands[i] != NULL) {
            tokens = parse(commands[i++], &is_bg);
            flag = execute(tokens, is_bg);
            free(tokens);
        }
        free(commands);
    } else {
        do {
            printf("%s", "sbush> ");
            command = get_command();
            tokens = parse(command, &is_bg);
            flag = execute(tokens, is_bg);
            is_bg = 0;
            free(command);
            free(tokens);
        } while (flag);
    }
}

int main(int argc, char* argv[]) {
    lifetime(argc, argv);

    return 0;
}
