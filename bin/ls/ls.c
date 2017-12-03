#include <unistd.h>
#include <stdio.h>
#include <sys/dirent.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 256

int main(int argc, char *argv[], char *envp[]) {
    int i;
    char buf[BUFSIZE];
    getcwd(buf, BUFSIZE);

    DIR * dir = opendir(buf);
    if (dir == NULL) {
        puts("Directory does not exist");
    }

    dentry* curr_dentry = NULL;
    while ((curr_dentry = readdir(dir)) != NULL) {
        for (i = 0; i < strlen(curr_dentry->name); i++) {
            putchar(curr_dentry->name[i]);
        }
        putchar(' ');
    }
    putchar('\n');
    closedir(dir);

    exit(0);
}
