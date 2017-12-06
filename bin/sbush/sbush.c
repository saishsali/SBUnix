#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/dirent.h>
#include <sys/mman.h>
#include <sys/paging.h>
#include <sys/process.h>
#define BUFSIZE 512

char **env;
char *ps1;
int num_env = 16;

char *getenv(const char *name) {
    int key_length = strlen(name);
    char initial_envp[BUFSIZE], *result = NULL;
    int i, j;

    if (name == NULL || env == NULL)
        return NULL;

    for (i = 0; env[i] != NULL; i++) {
        j = 0;
        while (j < key_length) {
            initial_envp[j] = env[i][j];
            j++;
        }
        initial_envp[key_length] = '\0';
        if (strcmp(name, initial_envp) == 0) {
            for (j = 0; env[i][j] != '\0'; j++) {
                if(env[i][j] == '=') {
                    result = env[i] + j + 1;
                }
            }
            break;
        }
    }
    return result;

}

void update_envp(const char *name, const char *value) {
    int i, j, key_length = strlen(name);
    char initial_envp[BUFSIZE];
    long int k;

    for (i = 0; env[i] != NULL; i++) {
        j = 0;
        while (j < key_length) {
            initial_envp[j] = env[i][j];
            j++;
        }
        initial_envp[key_length] = '\0';

        if (strcmp(name, initial_envp) == 0) {
            for (j = 0; env[i][j] != '\0'; j++) {
                if (env[i][j] == '=') {
                    j++;
                    for (k = j; env[i][k] != '\0'; k++) {
                        env[i][k] = '\0';
                    }
                    for(k = 0; k < strlen(value); k++) {
                        env[i][j+k] = value[k];
                    }
                    env[i][j+k] = '\0';
                    break;
                }
            }
        }
    }
}

void set_envp(const char *name, const char *value) {
    int i = 0;
    char equal[2] = "=", new_env[BUFSIZE];

    strcpy(new_env, name);
    strcat(new_env, equal);
    strcat(new_env, value);

    for (i = 0; env[i] != NULL; i++);
    if (i < num_env) {
        env[i] = (char *)malloc(256 * sizeof(char));
        strcpy(env[i], new_env);
        env[i + 1] = NULL;
    }
}

void setenv(const char *name, const char *value, int overwrite) {
    if (strlen(getenv(name)) > 0) {
        if (overwrite == 1) {
            update_envp(name, value);
        }
    } else {
        set_envp(name, value);
    }
}

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
int decode_environment_variable(char *var, char decoded_var[]) {
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
            if(value == NULL)
                return 0;
            strcat(decoded_var, value);
            j = j + strlen(value);
        } else {
            decoded_var[j++] = *var;
            var++;
        }
    }
    decoded_var[j] = '\0';
    return 1;
}


// Set environment variable
int set_environment_variable(char *token) {
    char decoded_var[BUFSIZE];
    char *name = strtok(token, "=");
    int result = decode_environment_variable(trim_quotes(strtok(NULL, "=")), decoded_var);
    if (result == 0)
        return 1;
    setenv(name, decoded_var, 1);

    if (strcmp(name, "PS1") == 0)
        strcpy(ps1, decoded_var);

    return 1;
}

// Change directory
int change_directory(char **tokens) {
    char path[100];

    if(tokens[1] == NULL) {
        strcpy(path, "/");
    } else {
        strcpy(path, tokens[1]);
    }
    if (chdir(path) != 0) {
        puts("\nError changing directory");
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
            token = strtok(NULL, "");
            if(token == NULL) {
                break;
            }
            tokens[i++] = token;
            break;
        }
        command = NULL;
    }

    tokens[i] = NULL;

    if (i > 0 && strcmp(tokens[i-1], "&") == 0) {
        *is_bg = 1;
        tokens[i - 1] = NULL;
    } else if ((i > 0 && tokens[i - 1][strlen(tokens[i - 1]) - 1] == '&')) {
        *is_bg = 1;
        tokens[i - 1][strlen(tokens[i - 1]) - 1] = '\0';
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

void print_path_variables() {
    int i;
    for (i = 0; env[i] != NULL; i++) {
        puts(env[i]);
    }
}

// Check for builtin commands
int builtin_command(char **tokens) {
    if (strcmp(tokens[0], "export") == 0) {
        if (tokens[1] == NULL) {
            print_path_variables();
            return 1;
        }
        return set_environment_variable(tokens[1]);
    } else if (strcmp(tokens[0], "cd") == 0) {
        return change_directory(tokens);
    } else if (strcmp(tokens[0], "exit") == 0) {
        shutdown();
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
                exit(1);
            }
        } else if (pid < 0) {
            puts("Fork error");
        } else {
            if (is_bg == 0) {
                waitpid(pid, NULL, 0);
            } else {
                yield();
            }
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
            if (n == -1) {
                puts("Command failed");
            }

            get_command(command, sizeof(command));
            parse(command, &is_bg, tokens);
            flag = execute(tokens, is_bg);
            is_bg = 0;
        } while (flag);
    }
}

/* Setup environment variables by making a copy of envp */
void setup_environment_variables(char *envp[]) {
    int i = 0;
    env = (char **)malloc((num_env + 1) * sizeof(char *));

    while (envp[i] != NULL && i < num_env) {
        env[i] = (char *)malloc(256 * sizeof(char));
        strcpy(env[i], envp[i]);
        i++;
    }
    env[i] = NULL;
}

int main(int argc, char* argv[], char *envp[]) {
    setup_environment_variables(envp);
    lifetime(argc, argv);

    return 0;
}
