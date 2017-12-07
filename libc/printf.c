#include <stdio.h>
#include <sys/stdarg.h>
#define SIZE 256

char *decimal_conversion(unsigned long long decimal, int base) {
    static char buf[SIZE];
    char *result = &buf[SIZE - 1], representation[] = "0123456789ABCDEF";
    int remainder;

    if (decimal == 0) {
        return "0";
    }

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
                    putchar(va_arg(arguments, int));
                    break;

                case 'd':
                    num = va_arg(arguments, int);
                    if (num < 0) {
                        putchar('-');
                        num *= -1;
                    }
                    printf(decimal_conversion(num, 10));
                    break;

                case 'x':
                    printf(decimal_conversion(va_arg(arguments, unsigned long long), 16));
                    break;

                case 's':
                    printf(va_arg(arguments, char*));
                    break;

                case 'p':
                    printf("0x");
                    printf(decimal_conversion(va_arg(arguments, unsigned long long), 16));
                    break;

                default:
                    // If no format specifier is matched, write as it is
                    putchar(*c);
                    putchar(*fmt);
            }
        } else {
            putchar(*fmt);
        }
        fmt++;
    }
}

// Write string pointer by fmt to standard output
int printf(const char *fmt, ...)
{
    va_list arguments;
    va_start(arguments, fmt);
    output(fmt, arguments);
    va_end(arguments);
    return 1;
}