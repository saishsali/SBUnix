#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *getenv(const char *name, char *envp[])
{
    int key_length = strlen(name);
    char initial_envp[1024], *result = " ";
    int i, j;

    if(name == NULL || envp == NULL)
        return NULL;
    
    for(i=0; envp[i] != NULL; i++)
    {
        strcpy(initial_envp,envp[i]);
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

        }
    }
    return result ; 
}