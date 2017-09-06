#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

void readdir(const char *name)
{
    int bpos;
    struct dirent *current_direct;
    int fd = open(name, 0x0000);
    char buf[1024];
    // int nread = syscall(SYS_getdents, fd, buf, 1024);
    int nread = getdents(fd , buf, 1024);
    current_direct = (struct dirent *) (buf);


    for (bpos = 0; bpos < nread;) {
        putchar(bpos+'0');
        current_direct = (struct dirent *) (buf+bpos);
        puts(current_direct->d_name);
        // printf("%4d %10lld  %s\n", current_direct->d_reclen,
        //                    (long long) current_direct->d_off, current_direct->d_name);
        // printf("d_reclen %d ---> \n", current_direct->d_reclen);
        bpos += current_direct->d_reclen;
    }
    close(fd);
    return ;
}
