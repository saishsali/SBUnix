#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define BUFSIZE 1024

void readdir(const char *name)
{
    int buffer_position, read_length, fd, k;
    char dir_buff[BUFSIZE];
    dirent *current_directory;
    fd = open(name, 0x0000);

    read_length = getdents(fd , dir_buff, 1024);

    for (buffer_position = 0; buffer_position < read_length;) {
        current_directory = (dirent *) (dir_buff + buffer_position);

        if (current_directory->d_name[0] != '.' && strcmp(current_directory->d_name, "..") != 0) {

            for (k = 0; k < strlen(current_directory->d_name); k++) {
                putchar(current_directory->d_name[k]);
            }
            putchar(' ');
            putchar(' ');
        }
        buffer_position += current_directory->d_reclen;
    }

    close(fd);
    putchar('\n');
}
