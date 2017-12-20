#include <unistd.h>
#include <sys/defs.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    pid_t pid;
    int status;
    char *environment[] = {"PATH=/rootfs/bin", "PS1=sbush> "};
    char *arguments[] = {"/rootfs/bin/sbush", "/rootfs/etc/rc", NULL};

    pid = fork();
    if (pid == 0) {
        if (execvpe("/rootfs/bin/sbush", arguments, environment) < 0) {
            printf("-sbush: command not found");
            exit(1);
        }
    } else {
        waitpid(pid, NULL);
    }

    pid = fork();
    if (pid == 0) {
        if (execvpe("/rootfs/bin/sbush", NULL, environment) < 0) {
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

