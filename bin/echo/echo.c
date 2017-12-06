#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char **env;

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

        if(value)
            puts(value);
        else
            putchar('\n');

    } else {
        sanitize(name);
        puts(name);
    }

    return 1;
}

int main(int argc, char *argv[], char *envp[]) {
    env = envp;
    char str[256];
    int len, i, index = 1, c = 0;

    if (argv[1] == NULL) {
        puts("\n");
    } else {
        if(argv[1][0] == '$') {
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
    }

    exit(0);
}
