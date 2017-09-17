#include <sys/kprintf.h>
#include <sys/io.h>
#include <sys/keyboard_scancode.h>
#define initial_pos_y 77
#define pos_x 24
#define control_scan_code 29
static int print_pos = initial_pos_y;

void keyboard_interrupt() {
	int x = inb(0x60);
	char c;

	if (x <= 127) {
		c = kbdus[inb(0x60)];
		if (x == control_scan_code){
			// Print first ^ at 77 and make next pointer to point at 78
			kprintf_pos(pos_x, initial_pos_y, "^");
			print_pos = initial_pos_y + 1;
		} else {
			kprintf_pos(pos_x, print_pos, "%c", c);
			kprintf_pos(pos_x, print_pos + 1, " ");
			print_pos = initial_pos_y;
		}
	}
}
