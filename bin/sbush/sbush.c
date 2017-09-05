#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFSIZE 1024

char *start = '\0';

char* tokenizer(char* str, const char* delim) {
    int i = 0;
    int len = strlen(delim);

    if(!str && !start)
        return '\0';

    if(str && start == '\0') {
        start = str;
    }

    char* token = start;
    while (1) {
        for (i = 0; i < len; i++) {
            if (*token == delim[i]) {
                token++;
                break;
            }
        }

        if (i == len) {
            start = token;
            break;
        }
    }

    if (*start == '\0') {
        start = '\0';
        return '\0';
    }

    while (*start != '\0') {
        for (i = 0; i < len; i++) {
            if (*start == delim[i]) {
                *start = '\0';
                break;
            }
        }
        start++;
        if (i < len)
            break;
    }

    if (*start == '\0')
        start = '\0';

    return token;
}

char *trim_quotes(char *str) {
    if (str[0] == '\"' || str[0] == '\'') {
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

void decode_environment_variable(char *var, char decoded_var[]) {
    int i = 0, j = 0;

    while(*var) {
        if(*var == '$' && (is_alphabet(*var + 1) == 1)) {
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

int set_environment_variable(char *token) {
    char decoded_var[BUFSIZE];
    char *name = tokenizer(token, "=");
    decode_environment_variable(trim_quotes(tokenizer(NULL, "=")), decoded_var);
    setenv(name, decoded_var, 1);

    return 1;
}

int get_environment_variable(char *name) {
    if(name[0] == '$') {
        name++;
        puts(getenv(name));
    } else {
        puts(name);
    }

    return 1;
}

int change_directory(char **tokens) {
    if (chdir(tokens[1]) != 0) {
        puts("Error changing directory");
    }

    return 1;
}


char *get_command() {
    size_t len = 0;
    ssize_t n;
    char *command;

    if ((n = getline(&command, &len, stdin)) == -1) {
        exit(EXIT_FAILURE);
    }
    command[n - 1] = '\0';

    return command;
}

void parse(char *command, int *is_bg, char *tokens[]) {
    int i = 0;
    char *token;

    while ((token = tokenizer(command, " \t\r\n")) != NULL) {
        tokens[i++] = token;
        // Special case when export statement has extra spaces in it
        if (strcmp(token, "export") == 0) {
            tokens[i++] = tokenizer(NULL, "");
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
    int num_of_cmnds = 0, i = 0, j = 0, iterate = 0, pipe1[2], pipe2[2];
    pid_t pid, cpid;
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

        //pipe1 is for odd command and pipe2 is for even command
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
            if (execvp(command[0], command) == -1) {
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
            while((cpid = waitpid(pid, NULL, 0)) > 0);
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
                puts("-sbush: command not found\n");
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

int open_script(char *filename) {
    int fd;

    fd = open(filename, 0x0000);
    if (fd < 0)
        exit(EXIT_FAILURE);

    return fd;
}

int close_script(int fildes) {
    return close(fildes);
}

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

void lifetime(int argc, char* argv[]) {
    char *tokens[BUFSIZE], *command, *ps1;
    int flag = 0, is_bg = 0, fd;
    setenv("PS1", "sbush> ", 1);

    if (argc >= 2) {
        fd = open_script(argv[1]);
        execute_script(fd);
        close_script(fd);
    } else {
        do {
            ps1 = getenv("PS1");
            while (*ps1) {
                putchar(*ps1);
                ps1++;
            }
            command = get_command();
            parse(command, &is_bg, tokens);
            flag = execute(tokens, is_bg);
            is_bg = 0;
        } while (flag);
    }
}

int main(int argc, char* argv[], char *envp[]) {
    lifetime(argc, argv);

    return 0;
}
