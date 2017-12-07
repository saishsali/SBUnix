#include <sys/wait.h>
#include <unistd.h>
#include <sys/defs.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
    pid_t pid;

    pid = fork();
    if (pid == 0) {
        execvpe("/rootfs/bin/sbush", NULL, envp);
    } else {
        waitpid(pid, NULL);
    }
    return 1;
}

