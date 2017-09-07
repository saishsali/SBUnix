#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BUFSIZE 2048

char *getenv(const char *name, char *envp[])
{
    int key_length = strlen(name);
    char initial_envp[BUFSIZE], *result = "\0";
    int i, j;

    if(name == NULL || envp == NULL)
        return NULL;

    for(i=0; envp[i] != NULL; i++) {
        j = 0;
        while (j < key_length) {
            initial_envp[j] = envp[i][j];
            j++;
        }
        initial_envp[key_length] = '\0';
        if(strcmp(name, initial_envp)==0)
        {
            for(j=0;envp[i][j]!='\0';j++)
            {
                if(envp[i][j] == '=')
                {
                    result = envp[i]+j+1;
                }
            }
            break;
        }
    }

    return result;
}
