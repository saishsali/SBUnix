#include <sys/kprintf.h>
#define POS_X 24
#define POS_Y 70
#define FREQUENCY 18

int i = 0, timer = 0;

void timer_interrupt() {
    timer++;
    if (timer % FREQUENCY == 0) {
        kprintf_pos(POS_X, POS_Y, "%d", ++i);
    }
}
