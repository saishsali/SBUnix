#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
	char cwd[BUFSIZE];
	DIR *curr_dir;
	struct dirent *current_direct;
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		curr_dir = opendir (cwd);
		if (curr_dir == NULL) {
			printf ("Cannot open directory - %s\n", cwd);
			exit(EXIT_FAILURE);
		}

		while ((current_direct = readdir(curr_dir)) != NULL) {
			printf ("[%s] ", current_direct->d_name);
		}
		printf("\n");
		closedir (curr_dir);
	}        
	else
		perror("Something went wrong");
	return 0;
}
