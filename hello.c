#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main ( ) 
{
    pid_t pid;
    int i = 0, c, j = 0, pos = 0;
    char *path = getenv("PATH");
    char *envp[] = { path, NULL };
    int link[2];
    if (pipe(link) == -1){
        exit(1);
    }
    while(i<5) {
        i++;
        j=0;
        pos=0;
        char *line = (char*) malloc(100*sizeof(char));
        char **line1 = (char**) malloc(20*sizeof(char*));
        char *token = (char*) malloc(20*sizeof(char));
        
        printf(">");
        while( ( c = getchar() )!=EOF && c != '\n' ) {
            line[pos] = c;
            pos++;
        }
        token = strtok(line, " ");
        while(token != NULL) {
            line1[j] = token;
            token = strtok(NULL, " ");
            j++;
        }
        line1[j] = NULL;
        pid = fork();
        if (pid==0) {
            execvpe(line1[0], line1, envp);
            exit(1);
        }
        else {
            waitpid(pid, 0, 0);
        }
        i++;
        free(line);
        free(line1);
        free(token);
    }
    

    return 0;
}
