#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    putchar('f');
    int fds[2];
    pipe(fds);
    dup2(fds[1], 1);
    putchar('x');
    char output[1000];
    size_t size = 100;
    chdir("/home/cmehndiratta/");
    getcwd(output,size);
    int k;
    for(k=0;k<5;k++) {
    	putchar(output[k]);
    }
	// char str[100];
    // char *s = (char*)malloc(10);
    // s[0] = 's';

    // putchar(s[0]);

    return 0;
}
