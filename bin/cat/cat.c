#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    int c, i;
    char *input;
    ssize_t n;
    size_t len = 0;

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            fp = fopen(argv[i], "r");

            if (fp == NULL) {
                printf("cat: %s: Cannot open file\n", argv[i]);
                exit(EXIT_FAILURE);
            }

            while ((c = fgetc(fp)) != EOF)
                putchar(c);

            fclose(fp);
        }
    } else {
        while ((n = getline(&input, &len, stdin)) != -1) {
            printf("%s", input);
        }
    }

    return 0;
}
