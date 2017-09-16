#include <sys/kprintf.h>
#include <sys/memcpy.h>
#include <sys/stdarg.h>
#define DEFAULT_COLOR 7
#define VIDEO_MEM_START 0xb8000
#define VIDEO_MEM_END 0xb8fa0
#define ROW_SIZE 25
#define COLUMN_SIZE 160

char *video_memory = (char *)VIDEO_MEM_START;

void print_character(char value, int color) {
    char blank[COLUMN_SIZE] = {' '};

    // Scroll functionality
    if (video_memory >= (char *)VIDEO_MEM_END) {
        video_memory = (char *)VIDEO_MEM_START;
        video_memory = memcpy((char*)video_memory, (char*)(video_memory + COLUMN_SIZE), ROW_SIZE * COLUMN_SIZE - COLUMN_SIZE);
        video_memory = (char*)(VIDEO_MEM_END - COLUMN_SIZE);
        video_memory = memcpy((char*)video_memory, blank, COLUMN_SIZE);
    }

    if (value == '\n') {
        // Move to the start of next row
        video_memory += ((int)((char*)VIDEO_MEM_END - video_memory) % COLUMN_SIZE);
    } else if (value == '\r') {
        // Move to the start of the current row
        video_memory -= ((int)(video_memory - (char*)VIDEO_MEM_START) % COLUMN_SIZE);
    } else {
        *video_memory = value;
        video_memory++;
        *video_memory = color;
        video_memory++;
    }
}

char *convert_decimal(unsigned int decimal, int base) {
    static char buf[100];
    char *result = &buf[99], representation[] = "0123456789ABCDEF";
    int remainder;

    while (decimal > 0) {
        remainder = decimal % base;
        decimal /= base;
        result--;
        *result = representation[remainder];
    }

    return result;
}

// Parse and output to console
void output(const char *fmt, va_list arguments) {
    int i;
    char c, *arg_value;
    const char *iter;
    unsigned long pointer_arg_value;

    for(iter = fmt; *iter; iter++) {
        if (*iter == '%') {
            iter++;

            switch(*iter) {
                case 'c':
                    c = va_arg(arguments, int);
                    print_character(c, DEFAULT_COLOR);
                    break;

                case 'd':
                    i = va_arg(arguments, int);
                    if (i < 0) {
                        print_character('-', DEFAULT_COLOR);
                    }
                    arg_value = convert_decimal(i, 10);
                    kprintf(arg_value);
                    break;

                case 'x':
                    i = va_arg(arguments, unsigned int);
                    if (i < 0) {
                        print_character('-', DEFAULT_COLOR);
                    }
                    arg_value = convert_decimal(i, 16);
                    kprintf(arg_value);
                    break;

                case 's':
                    arg_value = va_arg(arguments, char*);
                    kprintf(arg_value);
                    break;

                case 'p':
                    pointer_arg_value = va_arg(arguments, unsigned int);
                    kprintf("0x");
                    if (pointer_arg_value == '\0') {
                        print_character('0', DEFAULT_COLOR);
                    }
                    kprintf(convert_decimal(pointer_arg_value, 16));
                    break;

                default:
                    kprintf("Format not supported");
            }
        } else {
            print_character(*iter, DEFAULT_COLOR);
        }
    }
}

void kprintf(const char *fmt, ...)
{
    va_list arguments;

    va_start(arguments, fmt);
    output(fmt, arguments);
    va_end(arguments);
}

// Print at a specific position on the console
void kprintf_pos(int row, int column, const char *fmt, ...) {
    char *vm_ptr = video_memory;
    va_list arguments;

    va_start(arguments, fmt);
    video_memory = (char *)VIDEO_MEM_START + row * COLUMN_SIZE + column * 2;
    output(fmt, arguments);
    va_end(arguments);
    video_memory = vm_ptr;
}
