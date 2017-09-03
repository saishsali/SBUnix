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

int isalphabet(char c) {
	if ((c >='a' && c <='z' ) || ( c >= 'A' && c <= 'Z'))
		return 1;
	return 0;
}

char *decode_dollar_variables(char *val) {
    char *text = val;
    char *return_value = (char*) malloc(1024 * sizeof(char));
    int i=0, j=0;
    while(*text) {
        if(*text == '$' && (isalphabet(*text+1))) {
            char *key = (char*) malloc(1024 * sizeof(char));
            text++;
            i=0;
            while(isalphabet(*text)) {
                key[i] = *text;
                text++;
                i++;
            }
            char *key_value = getenv(key);
            strcat(return_value, key_value);
            j = j + strlen(key_value);
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

char **parse(char *input, int *is_bg, int *input_length) {
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
    *input_length = i + 1;

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

void execute_pipes(char **tokens, int tokens_len) {
    int num_cmnds = 0, k, i=0, iterate=0;
    pid_t pid;
    int pipe1[2], pipe2[2];
    char **commands = (char**) malloc(100*sizeof(char*));

    for(k = 0; k < tokens_len-1; k++)
        if(strcmp(tokens[k], "|")==0)
            num_cmnds++;

    num_cmnds++;
    for(k = 0; k < tokens_len; k++) {
        i=0;
        while(k<tokens_len-1 && strcmp(tokens[k], "|") != 0) {
            commands[i++] = tokens[k++];
        }
        commands[i++] = NULL;

        // pipe1 or pipe2 depends on which pipe was active previously
        if(iterate & 1) {
            if(pipe(pipe1)==-1)
                perror("Something went wrong");
        }
        else {
            if(pipe(pipe2)==-1)
                perror("Something went wrong");
        }

        //pipe1 is for odd commands and pipe2 is for even commands
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
            if (execvp(commands[0], commands) == -1) {
                printf("-sbush: %s: command not found\n", commands[0]);
            }
        }
        else {
            if (iterate == 0){
                close(pipe2[1]);
            } else{
                if (iterate & 1){
                    close(pipe2[0]);
                        if(iterate != num_cmnds - 1)
                            close(pipe1[1]);
                }else{
                    close(pipe1[0]);
                        if(iterate != num_cmnds - 1)
                            close(pipe2[1]);
                }
            }
            waitpid(pid,NULL,0);
        }

        iterate++;
    }

}


int execute(char **tokens, int is_bg, int tokens_len) {
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
            return builtin_func(tokens);
        }
    }

    pipe_present = check_pipes(tokens);

    if (pipe_present == 1) {
        execute_pipes(tokens, tokens_len);
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
    int flag = 0, i = 0, is_bg = 0, tokens_len;

    if (argc >= 2) {
        commands = read_script(argv[1]);
        while (commands[i] != NULL) {
            tokens = parse(commands[i++], &is_bg, &tokens_len);
            flag = execute(tokens, is_bg, tokens_len);
        }
    } else {
        do {
            printf("%s", "sbush> ");
            command = get_command();
            tokens = parse(command, &is_bg, &tokens_len);
            flag = execute(tokens, is_bg, tokens_len);
            is_bg = 0;
        } while (flag);
        free(command);
    }

    // Free pointers
}

int main(int argc, char* argv[]) {
    lifetime(argc, argv);

    return 0;
}
