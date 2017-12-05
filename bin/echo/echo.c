#include <stdlib.h>
#include <stdio.h>

char **env;

// Get and print environment variable
int get_environment_variable(char *name) {
    char *value;
    if(name[0] == '$') {
        name++;
        value = getenv(name);
        if(value)
            puts(value);
        else
            putchar('\n');

    } else {
        puts(name);
    }

    return 1;
}

int main(int argc, char *argv[], char *envp[]) {
    env = envp;
    get_environment_variable(argv[1]);

    exit(0);
}
