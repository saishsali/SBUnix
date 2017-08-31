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
    pid = fork();
    int i=0, j=0;
    char *line=NULL;
    size_t bufsize = 1024;
    int d;
    char *path = getenv("PATH");
    char *line_copy=(char*) malloc(32*sizeof(char));
    char *envp[] = {path, NULL};
    char **line1 = (char**) malloc(32*sizeof(char*));

    while(i<5) {
        j=0;
        printf(">");
        d = getline(&line, &bufsize, stdin);
        strcpy(line_copy,line);
        char *token = strtok(line, " ");

        while(token!=NULL) {
            line1[j] = (char*) malloc(32*sizeof(char));
            strcpy(line1[j++], token);
            token = strtok(NULL, " ");
        }
        i++;
        if (pid==0) {
            if (execvpe(line1[0],line1,envp) == -1) {
             	printf("some error");
            }
             exit(127);
        }
        else {
             waitpid(pid,0,0);
        }
        
        FILE *ls = popen(line_copy, "r");

        char buf[256];
        printf("\n");
        while (fgets(buf, sizeof(buf), ls) != 0) {
            printf("%s",buf);
        }
        pclose(ls);

    }

    return d;
}
