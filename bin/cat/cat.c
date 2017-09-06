#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define EXIT_FAILURE 0
#define BUFSIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    int i, fd;
    char input[BUFSIZE], c;
    ssize_t n;
    size_t len = 0;

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            fd = open(argv[i], 0x0000);
            if (fd < 0)
                exit(EXIT_FAILURE);

            while ((n = read(fd, &c, 1) != 0))
                putchar(c);

            close(fd);
        }
    } else {
        while ((n = getline(input, &len)) != -1) {
            puts(input);
        }
    }

    return 0;
}
