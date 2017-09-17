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
	static int shift = 0;

	// Scancode of pressed key
	if (x <= 0x7F) {

		// Shift key
		if (x == 0x2A || x == 0x36) {
			shift = 1;

			return;
		} else if (x == CONTROL_SCAN_CODE) {
			kprintf_pos(POS_X, INITIAL_POS_Y, "^");
			print_pos = INITIAL_POS_Y + 1;

			return;
		}

		if (shift == 1) {
			c = shift_mappings[x];
			shift = 0;
		} else {
			c = kbdus[x];
		}
		// kprintf_pos(24, 78, "%c", c);
		kprintf_pos(POS_X, print_pos, "%c", c);
		kprintf_pos(POS_X, print_pos + 1, " ");
		print_pos = INITIAL_POS_Y;
	}
}
