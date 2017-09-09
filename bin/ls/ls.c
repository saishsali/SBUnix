#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#define BUFSIZE 1024

void readdir(int fd)
{
    int buffer_position, read_length, k;
    char dir_buff[BUFSIZE];
    dirent *current_directory;

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

    putchar('\n');
}

int main(int argc, char *argv[], char *envp[]) {
    char cwd[BUFSIZE];
    int fd;

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        fd = open(cwd, 0x0000, 444);
        readdir(fd);
        close(fd);
    }
    else
        puts("Command failed");

    return 0;
}
