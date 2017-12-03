#include <unistd.h>
#include <stdio.h>
#include <sys/dirent.h>
#include <string.h>
#include <fcntl.h>
#define BUFSIZE 1024

// void readdir1(int fd)
// {
//     int buffer_position, read_length, k;
//     char dir_buff[BUFSIZE];
//     dentry *current_directory;

//     read_length = getdents(fd , dir_buff, 1024);

//     for (buffer_position = 0; buffer_position < read_length;) {
//         current_directory = (dentry *) (dir_buff + buffer_position);

//         if (current_directory->name[0] != '.' && strcmp(current_directory->name, "..") != 0) {

//             for (k = 0; k < strlen(current_directory->name); k++) {
//                 putchar(current_directory->name[k]);
//             }
//             putchar(' ');
//             putchar(' ');
//         }
//         buffer_position += current_directory->d_reclen;
//     }

//     putchar('\n');
// }

int main(int argc, char *argv[], char *envp[]) {

    char buf[1024];
    getcwd(buf, 1024);

    DIR * dir = opendir(buf);
    if(dir == NULL) {
        puts("Directory does not exist");
    } else {
        puts("Directory exists");
    }

    int k;
    dentry* curr_dentry = NULL;
    while((curr_dentry = readdir(dir)) != NULL) {
        for (k = 0; k < strlen(curr_dentry->name); k++) {
            putchar(curr_dentry->name[k]);
        }
        putchar(' ');
    }
    closedir(dir);


    // char cwd[BUFSIZE];
    // int fd;

    // if (getcwd(cwd, sizeof(cwd)) != NULL) {
    //     fd = open(cwd, 0x0000);
    //     readdir1(fd);
    //     close(fd);
    // }
    // else
    //     puts("Command failed");

    return 0;
}
