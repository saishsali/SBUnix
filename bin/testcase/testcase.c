#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/dirent.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/paging.h>

#define BUFSIZE 1024

void read_test() {
    puts("\nRunning read test. Please type something");
    char buf[BUFSIZE];
    read(0, buf, 128);
    puts(buf);
    puts("\nRead successful");
}

void write_test() {
    puts("\nRunning write test");
    write(1, "Write successful", 9);
}

void opendir_test() {
    puts("Running opendir test");
    DIR * dir = opendir("/rootfs/etc/");
    if(dir == NULL) {
        puts("Directory does not exist");
    } else {
        puts("Directory exists");
    }
    puts("\nopendir successful");
}

void readdir_test(){
    puts("\nRunning readdir test");
    DIR * dir = opendir("/rootfs/bin/");
    if(dir == NULL) {
        puts("Directory does not exist");
    } else {
        puts("Directory exists");
    }

    int k;
    dentry* curr_dentry = NULL;
    while((curr_dentry = readdir(dir)) != NULL) {
        for (k = 0; k < strlen(curr_dentry->d_name); k++) {
            putchar(curr_dentry->d_name[k]);
        }
        putchar(' ');
    }
    closedir(dir);
    puts("\nReaddir successful");
}

void file_open_close_test() {
    puts("\nRunning readdir open and close file test");
    int fd = open("/rootfs/etc/test/script.sh", O_RDONLY);
    close(fd);
    printf("FD: %d\n", fd);
    puts("\nOpen and close file test successful");
}

void getcwd_chdir() {
    puts("\nRunning get current working directory test");
    char buf[BUFSIZE], buf1[BUFSIZE];
    getcwd(buf, BUFSIZE);
    puts(buf);

    chdir("/rootfs/bin/../etc");

    getcwd(buf1, BUFSIZE);
    puts(buf1);
    puts("\nGet current working directory test successful");
}

void malloc_test() {
    int i;
    puts("\nRunning malloc test");
    for (i = 0; i < 100; i++) {
        char *p = malloc(1000);
        strcpy(p, "Hello World");
        free(p);
    }
    puts("\nMalloc test executed successfully");
}


void file_read_test() {
    puts("\nRunning file read test");
    int fd = open("/rootfs/etc/test/script.sh", O_RDONLY);
    char buf[1024];
    read(fd, buf, 10);
    puts("reading file");
    puts(buf);
    read(fd, buf, 10);
    puts("reading file");
    puts(buf);
    puts("\nFile read test successful");
}

void fork_test1() {
    puts("\nRunning fork test 1");
    int pid = fork();
    if (pid == 0) {
        write(1, "\nChild 1", 2);
        exit(0);
    } else {
        write(1, "\nParent 1", 2);
        waitpid(pid, NULL);
    }
    puts("\nFork test 1 successful");
}

void fork_test2() {
    puts("\nRunning fork test 2");
    int pid = fork();
    if (pid == 0) {
        write(1, "\nChild 1", 2);
        exit(0);
    } else {
        write(1, "\nParent 1", 2);
        waitpid(pid, NULL);

        int pid2 = fork();
        if (pid2 == 0) {
            write(1, "\nChild 2", 2);
            exit(0);
        } else {
            write(1, "\nParent 2", 2);
            waitpid(pid, NULL);
        }
    }
    puts("\nFork test 2 successful");
}

void fork_test3() {
    puts("\nRunning fork test 3");
    char *p = mmap((void *)0x1000, 100, RW_FLAG); // 1 page
    strcpy(p, "Hello");
    puts("Before fork: ");
    puts(p);

    int pid = fork();
    if (pid == 0) {
        write(1, "\nChild 1", 2);
        p[0] = 'W';
        puts("In child: ");
        puts(p);
        exit(0);
    } else {
        write(1, "\nParent 1", 2);
        waitpid(pid, NULL);
        puts("In parent: ");
        puts(p);
    }
    puts("\nFork test 3 successful");
}

void sys_exit_test() {
    puts("\nRunning exit test");
    int pid = fork();
    if (pid == 0) {
        write(1, "\nChild 1", 2);
        exit(0);
    } else {
        write(1, "\nParent 1", 2);
        waitpid(pid, NULL);
    }
    puts("\nExit test successful");
}


void execvpe_test() {
    puts("\nRunning execvpe test");
    char *args[10] = {"/rootfs/bin/echo", "Hello World"};

    int pid = fork();
    if (pid == 0) {
        execvpe("/rootfs/bin/echo", args, NULL);
    } else {
        waitpid(pid, NULL);
    }
    puts("\nExecvpe test successful");
}


void fork_multiple_times_test() {
    puts("\nRunning fork multiple times test");
    char *s[10] = {"/rootfs/bin/echo", "Hello World"};
    int pid;
    int i;

    for(i = 0; i < 10; i++) {
        printf("\n");
        pid = fork();
        if(pid == 0) {
            execvpe("/rootfs/bin/echo", s, NULL);
        } else {
            waitpid(pid, NULL);
        }
    }
    puts("\nFork multiple times test successful");
}

int main(int argc, char *argv[], char *envp[]) {

    read_test();

    puts("\n************************");

    write_test();

    puts("\n************************");

    opendir_test();

    puts("\n************************");

    readdir_test();

    puts("\n************************");

    file_open_close_test();

    puts("\n************************");

    getcwd_chdir();

    puts("\n************************");

    malloc_test();

    puts("\n************************");

    file_read_test();

    puts("\n************************");

    fork_test1();

    puts("\n************************");

    fork_test2();

    puts("\n************************");

    fork_test3();

    puts("\n************************");

    sys_exit_test();

    puts("\n************************");

    execvpe_test();

    puts("\n************************");

    fork_multiple_times_test();

    puts("\n************************");

    return 0;
}
