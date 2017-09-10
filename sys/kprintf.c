#include <sys/kprintf.h>
#include <stdarg.h>
static char *temp2 = (char*)0xb8000;

void print_char(char value, int color) {
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

void print_string(char *arg_value) {
	while(*arg_value != '\0') {
		print_char(*arg_value, 7);
		arg_value++;
	}
}

char *convert_value(int decimal, int base) {
	static char buf[100];
	char *result = &buf[99];
	*result = '\0';

	char alphabets[] = "0123456789ABCDEF";
	int remainder;

	while(decimal > 0) {
		remainder = decimal % base;
		decimal /= base;
		result--;
		*result = alphabets[remainder];
	}
	return result;
}

void kprintf(const char *fmt, ...)
{
	va_list arguments;
	va_start(arguments, fmt);
	int i;
	char c;
	char *arg_value;
	const char *iter;

	for(iter = fmt; *iter; iter++) {
		if (*iter == '%') {
			iter++;

			switch(*iter) {
				case 'c':
					c = va_arg(arguments, int);
					print_char(c, 7);
					break;
				case 'd':
					i = va_arg(arguments, int);
					if (i < 0){
						print_char('-', 7);
					}
					arg_value = convert_value(i,10);
					print_string(arg_value);
					break;
				case 'x':
					i = va_arg(arguments, int);
					if (i < 0){
						print_char('-', 7);
					}
					arg_value = convert_value(i,16);
					print_string(arg_value);
					break;
				case 's':
					arg_value = va_arg(arguments, char*);
					print_string(arg_value);
					break;

			}
		} else {

			print_char(*iter, 3);
		}
	}

}