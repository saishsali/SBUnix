#include <string.h>

char *start = '\0';

char* strtok(char* str, const char* delim) {
    int i = 0;
    int len = strlen(delim);

    if(!str && !start)
        return '\0';

    if(str && start == '\0') {
        start = str;
    }

    char* token = start;
    while (1) {
        for (i = 0; i < len; i++) {
            if (*token == delim[i]) {
                token++;
                break;
            }
        }

        if (i == len) {
            start = token;
            break;
        }
    }

    if (*start == '\0') {
        start = '\0';
        return '\0';
    }

    while (*start != '\0') {
        for (i = 0; i < len; i++) {
            if (*start == delim[i]) {
                *start = '\0';
                break;
            }
        }
        start++;
        if (i < len)
            break;
    }

    if (*start == '\0')
        start = '\0';

    return token;
}
