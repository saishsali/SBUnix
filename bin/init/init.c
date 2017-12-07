#include <sys/wait.h>
#include <unistd.h>
#include <sys/defs.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    pid_t pid;
    int status;
    char *new_argv[] = {"/rootfs/bin/sbush", "/rootfs/etc/rc", NULL};

    pid = fork();
    if(pid == 0) {
        if (execvpe("/rootfs/bin/sbush", new_argv, envp) < 0) {
            printf("-sbush: command not found");
            exit(1);
        }
    } else {
        waitpid(pid, NULL);
    }

    pid = fork();
    if (pid == 0) {
        if (execvpe("/rootfs/bin/sbush", NULL, envp) < 0) {
            printf("-sbush: command not found");
            exit(1);
        }
    } else {
        while (1) {
            if ((int)wait(&status) < 0) {
                shutdown();
            }
        }
    }

    return 0;
}

