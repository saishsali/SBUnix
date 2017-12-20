#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

#define O_RDONLY 0
#define O_WRONLY 1

int open(const char *pathname, int flags);
int8_t close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(int *status);
int waitpid(int pid, int *status);

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
off_t lseek(int fd, off_t offset, int whence);
int mkdir(const char *pathname, mode_t mode);
int unlink(const char *pathname);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);

void yield();

void ps();

void shutdown();

#endif
