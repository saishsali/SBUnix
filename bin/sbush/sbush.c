#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define BUFSIZE 1024

// Global environment variable
char **env;

// Global ps1 variable
char *ps1;

// Removes leading and trailing quotes from a string
char *trim_quotes(char *str) {
    if (str[0] == '\"' || str[0] == '\'') {
        str++;
        str[strlen(str) - 1] = 0;
    }

    return str;
}

// Checks if a character is an alphabet
int is_alphabet(char c) {
    if ((c >='a' && c <='z' ) || ( c >= 'A' && c <= 'Z'))
        return 1;

    return -1;
}

// Decodes environment variable
void decode_environment_variable(char *var, char decoded_var[]) {
    int i = 0, j = 0;
    decoded_var[0] = '\0';

    while(*var) {
        if(*var == '$' && (is_alphabet(*(var + 1)) == 1)) {
            char name[BUFSIZE];
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
}


// Set environment variable
int set_environment_variable(char *token) {
    char decoded_var[BUFSIZE];
    char *name = strtok(token, "=");
    decode_environment_variable(trim_quotes(strtok(NULL, "=")), decoded_var);
    setenv(name, decoded_var, 1);

    if (strcmp(name, "PS1") == 0)
        strcpy(ps1, decoded_var);

    return 1;
}

// Get and print environment variable
int get_environment_variable(char *name) {
    char *value;
    if(name[0] == '$') {
        name++;
        value = getenv(name);
        if(value)
            puts(value);
        else
            putchar('\n');

    } else {
        puts(name);
    }

    return 1;
}

// Change directory
int change_directory(char **tokens) {
    if (chdir(tokens[1]) != 0) {
        puts("Error changing directory");
    }

    return 1;
}

// Get command input for sbush from stdin
void get_command(char command[], size_t len) {
    ssize_t n;
    if ((n = read(0, command, len)) == -1) {
        exit(0);
    }
    command[n - 1] = '\0';
}

// Parse command by using newlines, tabs, whitespaces tokens
void parse(char *command, int *is_bg, char *tokens[]) {
    int i = 0;
    char *token;

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
}

// Check if pipe exists in a command
int check_pipes(char **tokens) {
    int i = 0;

    while (tokens[i] != NULL) {
        if (strcmp(tokens[i++], "|") == 0)
            return 1;
    }

    return -1;
}

// Check for builtin commands
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

// Launch pipes
void execute_pipes(char **tokens) {
    int num_of_cmnds = 0, i = 0, j = 0, iterate = 0, pipe1[2], pipe2[2];
    pid_t pid;
    char *command[BUFSIZE];

    while (tokens[i] != NULL) {
        if(strcmp(tokens[i++], "|") == 0)
            num_of_cmnds++;
    }

    num_of_cmnds++;
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
                puts("Pipe failed");
        } else {
            if (pipe(pipe2) == -1)
                puts("Pipe failed");
        }

        //pipe1 is for even command and pipe2 is for odd command
        pid = fork();

        if(pid == 0) {
            if(iterate == 0) {
                dup2(pipe2[1], 1);
            } else {
                if (iterate & 1) { //odd
                    dup2(pipe2[0], 0);
                    if (iterate != num_of_cmnds - 1)
                        dup2(pipe1[1], 1);
                } else {
                    dup2(pipe1[0], 0);
                    if (iterate != num_of_cmnds - 1)
                        dup2(pipe2[1], 1);
                }
            }
            if (execvpe(command[0], command, env) == -1) {
                puts("-sbush: command not found\n");
            }
        } else {
            if (iterate == 0){
                close(pipe2[1]);
            } else {
                if (iterate & 1) {
                    close(pipe2[0]);
                    if (iterate != num_of_cmnds - 1)
                        close(pipe1[1]);
                } else {
                    close(pipe1[0]);
                    if (iterate != num_of_cmnds - 1)
                        close(pipe2[1]);
                }
            }
            waitpid(pid, NULL, 0);
        }

        if (tokens[i] == NULL)
            break;
        i++;
        iterate++;
    }
}


// Execute command
int execute(char **tokens, int is_bg) {
    pid_t pid;
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
            if (execvpe(tokens[0], tokens, env) < 0) {
                puts("-sbush: command not found");
            }
            exit(0);
        } else if (pid < 0) {
            puts("Fork error");
        } else {
            if (is_bg == 0)
                waitpid(pid, NULL, 0);
        }
    }

    return 1;
}

// Read file
int open_script(char *filename) {
    int fd;

    fd = open(filename, 0x0000);
    if (fd < 0)
        exit(0);

    return fd;
}

// Close file
int close_script(int fildes) {
    return close(fildes);
}

// Execute sbush script (files that start with #!rootfs/bin/sbush)
void execute_script(int fd) {
    int i = 0, comment = 0, is_bg = 0;
    ssize_t n;
    char c, command[BUFSIZE], *tokens[BUFSIZE];

    while ((n = read(fd, &c, 1) != 0)) {
        if (c == '#') {
            comment = 1;
        } else if (c == '\n') {
            if (comment == 1)
                comment = 0;
            else if (i != 0) {
                command[i] = '\0';
                i = 0;
                parse(command, &is_bg, tokens);
                execute(tokens, is_bg);
            }
        } else if (comment == 0) {
            command[i++] = c;
        }
    }
}

// Lifetime of a command
void lifetime(int argc, char* argv[]) {
    char command[BUFSIZE], *tokens[BUFSIZE];
    int flag = 0, fd, is_bg = 0;
    ssize_t n;

    setenv("PS1", "sbush> ", 1);
    ps1 = getenv("PS1");

    if (argc >= 2) { // If a script is to be executed
        fd = open_script(argv[1]);
        execute_script(fd);
        close_script(fd);
    } else {
        do {
            n = write(1, ps1, strlen(ps1));
            if (n == -1)
                puts("Command failed");

            get_command(command, sizeof(command));
            parse(command, &is_bg, tokens);
            flag = execute(tokens, is_bg);
            is_bg = 0;

        } while (flag);
    }
}

int main(int argc, char* argv[], char *envp[]) {
    env = envp;
    lifetime(argc, argv);

    return 0;
}
