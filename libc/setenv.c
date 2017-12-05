
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define BUFSIZE 256

extern char **env;

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
    env[i] = new_env;
    env[i+1] = NULL;
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
