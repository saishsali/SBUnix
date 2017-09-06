#include <sys/defs.h>
#include <stdlib.h>
#include <string.h>

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    int64_t status;
    int slash_check = 0, i = 0, truncated_path_len, j = 0, token_len;
    char *path_env, *token, *slash = "/";
    char truncated_path[1024];

    while (file[i] != '\0') {
        if (file[i++] == '/') {
            slash_check = 1;
            break;
        }
    }

    if (slash_check == 1) {
        __asm__ (
            "movq $59, %%rax;"
            "movq %1, %%rdi;"
            "movq %2, %%rsi;"
            "movq %3, %%rdx;"
            "syscall;"
            "movq %%rax, %0;"
            : "=r" (status)
            : "r" (file), "r" (argv), "r" (envp)
        );
    } else {
        status = 0;
        path_env = getenv("PATH", envp);
        token = strtok(path_env, ":");
        truncated_path_len = strlen(slash) + strlen(file);

        while (token != NULL) {
            i = strlen(token) + 1;
            token_len = i;
            j = 0;
            while (j < truncated_path_len) {
                truncated_path[j++] = token[i++];
            }
            strcat(token, slash);
            strcat(token, file);

            __asm__ (
                "movq $59, %%rax;"
                "movq %1, %%rdi;"
                "movq %2, %%rsi;"
                "movq %3, %%rdx;"
                "syscall;"
                "movq %%rax, %0;"
                : "=r" (status)
                : "r" (token), "r" (argv), "r" (envp)
            );

            j = 0;
            i = token_len;
            token[i-1] = '\0';
            while (j < truncated_path_len) {
                token[i++] = truncated_path[j++];
            }

            token = strtok(NULL, ":");
        }
    }
    return status;
}
