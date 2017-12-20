#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/dirent.h>
#define BUFSIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    int i, fd;
    char c;
    ssize_t n;

    if (argc == 1) {
        return 0;
    }

    for (i = 1; i < argc; i++) {
        fd = open(argv[i], O_RDONLY);
        if (fd < 0)
            exit(1);
        while ((n = read(fd, &c, 1) != 0)) {
            putchar(c);
        }

        close(fd);
    }

    return 0;
}
