#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUFSIZE 512

char **env;

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

void sanitize(char name[256]) {
    char new_name[256];
    int len = strlen(name), c = 0, i;

    if (name[0] == '\"' && name[len - 2] == '\"' ) {
        for (i = 1; i < len - 2; i++) {
            new_name[c++] = name[i];
        }
        strcpy(name, new_name);
    }
}

// Get and print environment variable
int get_environment_variable(char *name) {
    char *value;
    if (name[0] == '$') {
        name++;
        value = getenv(name);

        if (value) {
            printf("%s", value);
        }

    } else {
        sanitize(name);
        printf("%s", name);
    }

    return 1;
}

int main(int argc, char *argv[], char *envp[]) {
    env = envp;
    char str[256];
    int len, i, index = 1, c = 0;

    if (argv[1] == NULL) {
        return 0;
    }

    if (argv[1][0] == '$') {
        get_environment_variable(argv[1]);

    } else {
        while (index < argc) {
            len = strlen(argv[index]);
            for (i = 0; i < len; i++) {
                str[c++] = argv[index][i];
            }
            str[c++] = ' ';
            index++;
        }
        get_environment_variable(str);
    }

    exit(0);
}
