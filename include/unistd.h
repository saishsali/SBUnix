#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

int8_t close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
pid_t wait(int *status);

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
off_t lseek(int fd, off_t offset, int whence);
int mkdir(const char *pathname, mode_t mode);
int unlink(const char *pathname);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);

int dup2(int oldfd, int newfd);

void yield();
void ps();
int kill(pid_t pid, int sig);
void shutdown();

#endif
