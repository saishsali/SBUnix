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
	char seconds[10];
	int len;

	if (argc == 1) {
		printf("Usage: sleep <seconds>");
		return 1;
	}

	if (valid(argv[1]) == 0) {
		printf("Invalid arguments : Usage: sleep <seconds>\n");
		return 1;
	}

	len = strlen(argv[1]);
	if (argv[1][len - 1] == '&') {
		strcpy(seconds, argv[1]);
		seconds[len - 1] = '\0';

	} else {
		strcpy(seconds, argv[1]);
	}

    sleep(char_to_int(seconds));

    exit(0);
}