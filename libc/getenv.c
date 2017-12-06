#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BUFSIZE 256

extern char **env;

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
