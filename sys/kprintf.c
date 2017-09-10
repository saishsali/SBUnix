#include <sys/kprintf.h>
#include <stdarg.h>
#define DEFAULT_COLOR 7

char *temp2 = (char*)0xb8000;

void print_character(char value, int color) {
	if (value == '\n') {
		int y = (int)((char*)0xb8fa0 - temp2);
		y = y%160;
		temp2 = temp2 + y;
		return;
	}
	*temp2 = value;
	temp2++;
	*temp2 = color;
	temp2++;
}

char *convert_decimal(int decimal, int base) {
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

void kprintf(const char *fmt, ...)
{
	int i;
	char c, *arg_value;
	const char *iter;
	va_list arguments;

	va_start(arguments, fmt);
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
					pointer_arg_value = (unsigned long)va_arg(arguments, void*);
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
