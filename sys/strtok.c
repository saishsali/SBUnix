#include <sys/string.h>

char *start_pos = '\0';

char* strtok(char* str, const char* delim) {
    int i = 0;
    int len = strlen(delim);

    if(!str && !start_pos)
        return '\0';

    if(str && start_pos == '\0') {
        start_pos = str;
    }

    char* token = start_pos;
    while (1) {
        for (i = 0; i < len; i++) {
            if (*token == delim[i]) {
                token++;
                break;
            }
        }

        if (i == len) {
            start_pos = token;
            break;
        }
    }

    if (*start_pos == '\0') {
        start_pos = '\0';
        return '\0';
    }

    while (*start_pos != '\0') {
        for (i = 0; i < len; i++) {
            if (*start_pos == delim[i]) {
                *start_pos = '\0';
                break;
            }
        }
        start_pos++;
        if (i < len)
            break;
    }

    if (*start_pos == '\0')
        start_pos = '\0';

    return token;
}
