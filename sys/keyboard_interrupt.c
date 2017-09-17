#include <sys/kprintf.h>
#include <sys/io.h>
#include <sys/keyboard_scancode.h>
#define INITIAL_POS_Y 77
#define POS_X 24
#define CONTROL_SCAN_CODE 29
static int print_pos = INITIAL_POS_Y;

void keyboard_interrupt() {
	int x = inb(0x60);
	char c;

	if (x <= 127) {
		c = kbdus[inb(0x60)];
		if (x == CONTROL_SCAN_CODE){
			// Print first ^ at 77 and make next pointer to point at 78
			kprintf_pos(POS_X, INITIAL_POS_Y, "^");
			print_pos = INITIAL_POS_Y + 1;
		} else {
			kprintf_pos(POS_X, print_pos, "%c", c);
			kprintf_pos(POS_X, print_pos + 1, " ");
			print_pos = INITIAL_POS_Y;
		}
	}
}
