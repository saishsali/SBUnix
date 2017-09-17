#include <sys/kprintf.h>
#include <sys/io.h>
#include <sys/keyboard_scancode.h>

void keyboard_interrupt() {
	int x = inb(0x60);
	char c;

	if (x <= 127) {
		c = kbdus[inb(0x60)];
		kprintf_pos(24, 78, "%c", c);
	}
}
