#include <sys/kprintf.h>
int i = 0;
int timer = 0;

void timer_interrupt() {
    timer++;
    if (timer % 18 == 0) {
        kprintf_pos(24, 75, "%d", ++i);
    }
}
