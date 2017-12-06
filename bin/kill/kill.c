#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int char_to_int(char *s) {
    int pid = 0, i;
    for(i = 0; i < strlen(s); i++) {
        pid = pid * 10 + s[i] - '0';
    }
    return pid;
}

int main(int argc, char *argv[], char *envp[]) {
	int pid = char_to_int(argv[2]);
	if(pid > 2) {
		kill(pid);
	} else if(pid == 2 ) {
		// Sbush process
	} else {
		// Idle process cant be killed
		puts("\nCant kill this process");
	}
    exit(1);
}