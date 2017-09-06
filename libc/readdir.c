#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 1024

void readdir(const char *name)
{
    int bpos, nread, fd, k;
    char buf[BUFSIZE];
    dirent *current_direct;
    fd = open(nameg, 0x0000);

    nread = getdents(fd , buf, 1024);

    for (bpos = 0; bpos < nread;) {
        current_direct = (dirent *) (buf+bpos);
        if (current_direct->d_name[0] != '.' && strcmp(current_direct->d_name, "..") != 0) {
            for (k = 0; k < strlen(current_direct->d_name); k++) {
                putchar(current_direct->d_name[k]);
            }
            putchar(' ');
            putchar(' ');
        }
        bpos += current_direct->d_reclen;
    }
    close(fd);
    putchar('\n');
}
