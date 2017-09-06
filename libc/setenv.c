#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define BUFSIZE 2048

void update_envp(const char *name, const char *value, char *envp[]) {
    int i, j;
    long int k;
    for (i = 0; envp[i] != NULL; i++) {
        int key_length = strlen(name);
        char initial_envp[BUFSIZE];

        strcpy(initial_envp, envp[i]);
        initial_envp[key_length] = '\0';

        if (strcmp(name, initial_envp) == 0) {
            for (j = 0; envp[i][j] != '\0'; j++) {
                if (envp[i][j] == '=') {
                    j++;
                    for (k = j; envp[i][k] != '\0'; k++) {
                        envp[i][k] = '\0';
                    }
                    for(k = 0; k < strlen(value); k++) {
                        envp[i][j+k] = value[k];
                    }
                }
            }
            break;
        }
    }
}

void set_envp(const char *name, const char *value, char *envp[]) {
    int i = 0;
    char equal[2] = "=", new_env[BUFSIZE];

    strcpy(new_env, name);
    strcat(new_env, equal);
    strcat(new_env, value);

    for (i = 0; envp[i] != NULL; i++) {}
    envp[i] = new_env;
    envp[i+1] = NULL;
}


int setenv(const char *name, const char *value, int overwrite, char *envp[])
{
    if (strlen(getenv(name, envp)) > 0) {
        if (overwrite == 1)
            update_envp(name, value, envp);
    } else
        set_envp(name, value, envp);

    return 0;
}
