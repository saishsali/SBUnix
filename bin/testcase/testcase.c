#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/dirent.h>
#define BUFSIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    char *s[10] = {"/rootfs/bin/echo", "Hello World"};
    int pid;
    int i;

    for(i = 0; i < 500; i++) {
        pid = fork();
        if(pid == 0) {
            execvpe("/rootfs/bin/echo", s, envp);
        }
    }

    return 0;
}
