#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

char *builtins[] = {"cd", "export", "echo"};
int num_builtins() {
    return (sizeof(builtins)/sizeof(builtins[0]));
}

char *trim_quotes(char *val) {
    if(val[0]=='\"' || val[0]=='\'') {
        val++;
        val[strlen(val)-1] = 0;
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

void set_env(char *line) {
    char *key = strtok(line, "=");
    char *value = strtok(NULL, "=");
    value = trim_quotes(value);
    value = decode_dollar_variables(value);
    setenv(key, value, 1);
}

void get_env(char *line) {
    char *key = strtok(line, "=");
    key++;
    printf("%s\n", getenv(key));
}

void builtin_func(char **line) {
    if (strcmp(line[0], "export") == 0) {
        set_env(line[1]);
    }
    else if (strcmp(line[0], "echo") == 0) {
        get_env(line[1]);
    }
}

int main ( ) 
{
    pid_t pid, pid1;
    int i = 0, c, j = 0, pos = 0, builtin_i, is_builtin = 0, pipe_present=0;
    char *path = getenv("PATH");
    char *envp[] = { path, NULL };
    int link[2];
    if (pipe(link) == -1){
        exit(1);
    }

    while(i<15) {
        j=0;
        pos=0;
        is_builtin = 0;
        pipe_present = 0;
        char *line = (char*) malloc(200*sizeof(char));
        char **line1 = (char**) malloc(100*sizeof(char*));
        char *token = (char*) malloc(100*sizeof(char));
        
        printf(">");
        while( ( c = getchar() )!=EOF && c != '\n' ) {
            line[pos] = c;
            pos++;
        }
        line[pos] = '\0';

        if(strlen(line)==0) {
            continue;
        }

        token = strtok(line, " ");
        line1[j] = token;
        j++;
        // Special case when export statement has space in it
        if (strcmp(token, "export") == 0) {
            line1[j] = strtok(NULL, "");
            j++;
        }
        else {
            while((token = strtok(NULL, " ")) != NULL) {
                line1[j] = token;
                j++;
            }
            free(token);
        }
        
        line1[j] = NULL;
        
        //Check for builtin variables
        for(builtin_i = 0; builtin_i < num_builtins(); builtin_i++) {
            if (strcmp(line1[0], builtins[builtin_i]) == 0) {
                builtin_func(line1);
                is_builtin = 1;
            }
        }
        if(is_builtin == 0){
            int k;
            for(k=0; k<j; k++) {
                if(strcmp(line1[k], "|")==0) {
                    int fds[2];
                    char **part1 = line1;
                    part1[2] = NULL;
                    char **part2 = line1+k+1;

                    if(pipe(fds) == -1) {
                        perror("Something went wrong");
                        exit(1);
                    }
                    pid = fork(); 
                    if(pid==0) {
                        dup2(fds[1], 1);
                        close(fds[0]);
                        if(execvpe(part1[0], part1, envp) == -1) {
                            printf("-bash: %s: Command not found\n", part1[0]);
                        }
                    }
                    else {
                        pid1 = fork();
                        waitpid(pid, 0, 0);
                        if(pid1 == 0) {
                            dup2(fds[0], 0);
                            close(fds[1]);
                            if(execvpe(part2[0], part2, envp) == -1) {
                                printf("-bash: %s: Command not found\n", part2[0]);
                            }
                        }
                        
                    }
                    close(fds[0]);
                    close(fds[1]);
                    pipe_present = 1;

                    break;
                }
                

            }
            if (pipe_present == 0) {
                pid = fork(); 
                if (pid==0) {

                    if(execvpe(line1[0], line1, envp)==-1) {
                        printf("-bash: %s: Command not found\n", line1[0]);
                    }
                    exit(1);
                }
                else {
                    waitpid(pid, 0, 0);
                }
            }
        }
        
        i++;
        free(line);
        free(line1);

    }
    

    return 0;
}
