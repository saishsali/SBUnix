#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int char_to_int(char *s) {
    int pid = 0, i;
    for(i = 0; i < strlen(s); i++) {
        pid = pid * 10 + s[i] - '0';
    }
    return pid;
}

int main(int argc, char *argv[], char *envp[]) {
	kill(char_to_int(argv[1]));
    exit(1);
}