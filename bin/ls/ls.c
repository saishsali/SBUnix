#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#define BUFSIZE 1024

int main(int argc, char *argv[]) {
	char cwd[BUFSIZE];
	struct dirent *current_direct;
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		readdir(cwd);
	}
	else
		puts("Something went wrong");
	return 0;
}