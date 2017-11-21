#include <sys/dirent.h>

DIR* opendir(char *path) {
    DIR * ret_directory = NULL;
    __asm__ __volatile__(
        "movq $4, %%rax;"
        "movq %1, %%rdi;"
        "int $0x80;"
        "movq %%rax, %0;"
        : "=r" (ret_directory)
        : "r" (path)
        : "%rax", "%rdi"
    );
    return ret_directory;
}
