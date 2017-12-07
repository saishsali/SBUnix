#include <sys/wait.h>
#include <unistd.h>
#include <sys/defs.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        execvpe("/rootfs/bin/sbush", NULL, envp);
    } else {
        while (1) {
            if ((int)wait(&status) < 0) {
                shutdown();
            }
        }
    }

    return 0;
}

