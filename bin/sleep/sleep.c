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
	char seconds[10];
	// int flag = 1;
	// int len = strlen(argv[1]);
	// if (argv[1][len - 1] == '&') {
	// 	strcpy(seconds, argv[1]);
	// 	seconds[len - 1] = '\0';

	// } else if (argv[2] != NULL && argv[2][0] == '&') {
	// 	strcpy(seconds, argv[1]);

	// } else {
	// 	flag = 0;
	// }

    sleep(char_to_int(seconds));

    // if(flag == 1)
    // 	yield();

    // puts("going to exit");

    exit(1);
}