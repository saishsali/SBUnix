#include <stdlib.h>
#include <stdio.h>

char **env;

int main(int argc, char *argv[], char *envp[]) {
    env = envp;
    char *p;
    if ((p = getenv(argv[1])) != NULL) {
        puts(p);
    } else {
        puts(argv[1]);
    }

    exit(0);
}
