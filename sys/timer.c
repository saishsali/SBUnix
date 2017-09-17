#include <sys/kprintf.h>
#define pos_y 74
#define pos_x 24

int i = 0;
int timer = 0;

void timer_interrupt() {
    timer++;
    if (timer % 18 == 0) {
        kprintf_pos(pos_x, pos_y, "%d", ++i);
    }
}
