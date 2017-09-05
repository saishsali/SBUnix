#include <string.h>
char *start = 0;

char* tokenizer(char* str, const char* delim) {
    int i = 0;
    int len = strlen(delim);

    if(!str && !start)
        return NULL;

    if(str && !start)
        start = str;

    char* p_start = start;
    while (1) {
        for (i = 0; i < len; i++) {
            if (*p_start == delim[i]) {
                p_start++;
                break;
            }
        }

        if (i == len) {
            start = p_start;
            break;
        }
    }

    if (*start == '\0') {
        return NULL;
    }

    while (*start != '\0') {
        for (i = 0; i < len; i ++) {
            if (*start == delim[i]) {
                *start = '\0';
                break;
            }
        }

        start++;
        if (i < len)
            break;
    }

    return p_start;
}
