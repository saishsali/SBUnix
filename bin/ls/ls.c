#include <unistd.h>
#include <stdio.h>
#include <sys/dirent.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 256

void add_slash_at_end(char *path) {
    int i = strlen(path) - 1;
    if (path[i] != '/') {
        path[++i] = '/';
        path[++i] = '\0';
    }
}

int main(int argc, char *argv[], char *envp[]) {
    int i;
    char buf[BUFSIZE];

    if (argv[1]) {
        add_slash_at_end(argv[1]);
        strcpy(buf, argv[1]);
    } else {
        getcwd(buf, BUFSIZE);
    }

    DIR * dir = opendir(buf);
    if (dir == NULL) {
        exit(1);
    }

    dentry* curr_dentry = NULL;
    while ((curr_dentry = readdir(dir)) != NULL) {
        for (i = 0; i < strlen(curr_dentry->d_name); i++) {
            putchar(curr_dentry->d_name[i]);
        }
        putchar(' ');
    }

    closedir(dir);

    return 0;
}
