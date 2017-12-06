#include <sys/defs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BUFSIZE 512

char **env;

char *get_environment(const char *name) {
    int key_length = strlen(name);
    char initial_envp[BUFSIZE], *result = NULL;
    int i, j;

    if (name == NULL || env == NULL)
        return NULL;

    for (i = 0; env[i] != NULL; i++) {
        j = 0;
        while (j < key_length) {
            initial_envp[j] = env[i][j];
            j++;
        }
        initial_envp[key_length] = '\0';
        if (strcmp(name, initial_envp) == 0) {
            for (j = 0; env[i][j] != '\0'; j++) {
                if(env[i][j] == '=') {
                    result = env[i] + j + 1;
                }
            }
            break;
        }
    }
    return result;
}

int execvpe(const char *file, char *argv[], char *envp[]) {
    int64_t status = 0;
    char *token, *slash = "/";
    int slash_check = 0, i = 0;
    char absolute_path[256], path_env[256];

    env = envp;

    while (file[i] != '\0') {
        if (file[i++] == '/') {
            slash_check = 1;
            break;
        }
    }

    if (slash_check == 1) {
        __asm__ volatile(
            "movq $59, %%rax;"
            "movq %1, %%rdi;"
            "movq %2, %%rsi;"
            "movq %3, %%rdx;"
            "int $0x80;"
            "movq %%rax, %0;"
            : "=r" (status)
            : "r" (file), "r" (argv), "r" (envp)
            : "%rax", "%rdi", "%rsi", "%rdx"
        );
    } else {
        strcpy(path_env, get_environment("PATH"));
        token = strtok(path_env, ":");

        while (token != NULL) {
            strcpy(absolute_path, token);
            strcat(absolute_path, slash);
            strcat(absolute_path, file);

            __asm__ volatile(
                "movq $59, %%rax;"
                "movq %1, %%rdi;"
                "movq %2, %%rsi;"
                "movq %3, %%rdx;"
                "int $0x80;"
                "movq %%rax, %0;"
                : "=r" (status)
                : "r" (absolute_path), "r" (argv), "r" (envp)
                : "%rax", "%rdi", "%rsi", "%rdx"
            );
            token = strtok(NULL, ":");
        }
    }

    return status;
}
