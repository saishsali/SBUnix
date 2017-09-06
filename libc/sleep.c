#include <sys/defs.h>

unsigned int sleep(unsigned int seconds) {
    int64_t status;

    struct time {
       long tv_sec;
       long tv_nsec;
    };

    struct time req;
    struct time rem;
    req.tv_sec = seconds;
    req.tv_nsec = 0;

    __asm__ (
        "movq $35, %%rax;"
        "movq %1, %%rdi;"
        "movq %2, %%rsi;"
        "syscall;"
        "movq %%rax, %0;"
        : "=r" (status)
        : "r" (&req), "r" (&rem)
    );

    return status;
}
