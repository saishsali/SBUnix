#include <sys/kprintf.h>
#include <sys/string.h>
#include <sys/stdarg.h>
#define VIDEO_MEM_START 0xb8000
#define ROW_SIZE 25
#define COLUMN_SIZE 160
#define VIDEO_MEM_END (VIDEO_MEM_START + (ROW_SIZE - 1) * COLUMN_SIZE) // ROW_SIZE = 25th row is reserved for timer
#define SIZE 100
#define DEFAULT_COLOR 7

char *video_memory = (char *)VIDEO_MEM_START;
int scroll_flag = 1;

void scroll() {
    char *vm;

    // If current address exceeds video memory last address
    if (video_memory >= (char *)VIDEO_MEM_END) {
        video_memory = memcpy((char*)VIDEO_MEM_START, (char*)(VIDEO_MEM_START + COLUMN_SIZE), ROW_SIZE * COLUMN_SIZE - COLUMN_SIZE);
        video_memory = memset((char*)(VIDEO_MEM_END - COLUMN_SIZE), 0, COLUMN_SIZE);

        // Set Default color to white
        for (vm = (char*)(VIDEO_MEM_END - COLUMN_SIZE + 1); vm < (char*)(VIDEO_MEM_END); vm += 2)
            *vm = DEFAULT_COLOR;
        }
}

int control_character(char c) {
    switch (c) {
        case '\n': // Move to the start of the next line
            video_memory += ((int)((char*)VIDEO_MEM_END - video_memory) % COLUMN_SIZE);
            return 1;

        case '\r': // Move to the start of the current line
            video_memory -= ((int)(video_memory - (char*)VIDEO_MEM_START) % COLUMN_SIZE);
            return 1;

        case '\t': // Move one tab horizontal space
            video_memory += 8;
            return 1;

        default:
            return 0;
    }
}

void print_character(char c) {
    if (scroll_flag == 1)
        scroll();
    if (!control_character(c)) {
        *video_memory = c;
        video_memory += 2;
    }
}

char *decimal_conversion(unsigned int decimal, int base) {
    static char buf[SIZE];
    char *result = &buf[SIZE - 1], representation[] = "0123456789ABCDEF";
    int remainder;

    while (decimal > 0) {
        remainder = decimal % base;
        decimal /= base;
        result--;
        *result = representation[remainder];
    }

    return result;
}

// Handle format specifiers and write to console
void output(const char *fmt, va_list arguments) {
    int num;
    const char *c;

    while (*fmt) {
        if (*fmt == '%') {
            c = fmt;
            fmt++;

            switch(*fmt) {
                case 'c':
                    print_character(va_arg(arguments, int));
                    break;

                case 'd':
                    num = va_arg(arguments, int);
                    if (num < 0)
                        print_character('-');
                    kprintf(decimal_conversion(num, 10));
                    break;

                case 'x':
                    kprintf(decimal_conversion(va_arg(arguments, unsigned int), 16));
                    break;

                case 's':
                    kprintf(va_arg(arguments, char*));
                    break;

                case 'p':
                    kprintf("0x");
                    kprintf(decimal_conversion(va_arg(arguments, unsigned int), 16));
                    break;

                default:
                    // If no format specifier is matched, write as it is
                    print_character(*c);
                    print_character(*fmt);
            }
        } else {
            print_character(*fmt);
        }
        fmt++;
    }
}

// Write string pointer by fmt to standard output
void kprintf(const char *fmt, ...)
{
    va_list arguments;
    va_start(arguments, fmt);
    output(fmt, arguments);
    va_end(arguments);
}

// Write at a specific position to standard output
void kprintf_pos(int row, int column, const char *fmt, ...) {
    char *vm = video_memory;
    va_list arguments;
    scroll_flag = 0;
    va_start(arguments, fmt);
    video_memory = (char *)VIDEO_MEM_START + row * COLUMN_SIZE + column * 2;
    output(fmt, arguments);
    va_end(arguments);
    video_memory = vm;
    scroll_flag = 1;
}

// Clear entire screen and set default color to white
void clear_screen() {
    char *vm = (char *)VIDEO_MEM_START;
    memset(vm, 0, ROW_SIZE * COLUMN_SIZE);

    // Set Default color to white
    for (vm = (char*)(VIDEO_MEM_START + 1); vm < (char*)(VIDEO_MEM_START + ROW_SIZE * COLUMN_SIZE); vm += 2)
        *vm = DEFAULT_COLOR;
}
