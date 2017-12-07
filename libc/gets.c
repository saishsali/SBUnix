#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 512

char *gets(char *s) {
	read(0, s, BUFSIZE);
	return s;
}
