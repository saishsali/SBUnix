#include <sys/kprintf.h>
#include <sys/io.h>
#include <sys/keyboard_scancode.h>

void keyboard_interrupt() {
	static int i = 0;

	if (i == 0) {
		kprintf_pos(24, 78, "%c", kbdus[inb(0x60)]);
		i = 1;
	} else {
		inb(0x60);
		i = 0;
	}
}
