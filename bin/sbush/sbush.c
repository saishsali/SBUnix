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
        printf("Error changing directory");
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

void print_path_variables() {
    int i;
    for (i = 0; env[i] != NULL; i++) {
        printf(env[i]);
    }
}

int read_file(char *filename) {
    int fd;
    char c;
    ssize_t n;

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    while ((n = read(fd, &c, 1) != 0)) {
        putchar(c);
    }

    close(fd);

    return 1;
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
        exit(0);
    } else if (strcmp(tokens[0], "shutdown") == 0) {
        shutdown();
    } else if (strcmp(tokens[0], "ulimits") == 0) {
        return read_file("/rootfs/etc/ulimits");
    } else if (strcmp(tokens[0], "help") == 0) {
        return read_file("/rootfs/etc/help");
    }

    return -1;
}

// Execute command
int execute(char **tokens, int is_bg) {
    pid_t pid;
    int builtin;

    if (tokens[0] == NULL)
        return 1;

    if ((builtin = builtin_command(tokens)) != -1)
        return builtin;

    pid = fork();
    if (pid == 0) {
        if (execvpe(tokens[0], tokens, env) < 0) {
            printf("-sbush: command not found");
            exit(1);
        }
     } else if (pid < 0) {
        printf("Fork error");
    } else {
        if (is_bg == 0) {
            waitpid(pid, NULL);
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
                printf("%c", c);
                command[i] = '\0';
                i = 0;
                parse(command, &is_bg, tokens);
                execute(tokens, is_bg);
            }
        } else if (comment == 0) {
            command[i++] = c;
        }
    }

    if(c != '\n') {
        printf("\n");
        command[i] = '\0';
        parse(command, &is_bg, tokens);
        execute(tokens, is_bg);
    }
}

// Lifetime of a command
void lifetime(int argc, char* argv[]) {
    char command[BUFSIZE], *tokens[BUFSIZE];
    int flag = 0, fd, is_bg = 0;

    setenv("PS1", "sbush> ", 1);
    ps1 = getenv("PS1");

    if (argc >= 2) { // If a script is to be executed
        fd = open_script(argv[1]);
        execute_script(fd);
        close_script(fd);
    } else {
        do {
            printf("\n%s", ps1);
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
