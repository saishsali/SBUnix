#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int char_to_int(char *s) {
    int pid = 0, i;
    for(i = 0; i < strlen(s); i++) {
        pid = pid * 10 + s[i] - '0';
    }
    return pid;
}

int valid(char s[100]) {
	int i, len = strlen(s);

	for(i = 0; i < len; i++) {
		if((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z')) {
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[], char *envp[]) {

	if(strcmp(argv[1], "-9") != 0) {
		printf("Usage: kill -9 <pid>");
		return 1;
	}

	if (valid(argv[2]) == 0) {
		printf("Invalid arguments : Usage: kill -9 <pid>\n");
		return 1;
	}

	int pid = char_to_int(argv[2]);
	if (pid > 2) {
		kill(pid, 1);
	} else {
		// Idle process cant be killed
		printf("You are not allowed to kill this process\n");
	}
    exit(0);
}
