#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#define BUFSIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    char cwd[BUFSIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        readdir(cwd);
    }
    else
        puts("Command failed");
    return 0;
}
