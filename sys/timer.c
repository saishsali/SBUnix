#include <sys/kprintf.h>
#define ROW 24
#define COLUMN 30
#define FREQUENCY 18

int i = 0, timer = 0;

void timer_interrupt() {
    timer++;
    if (timer % FREQUENCY == 0) {
        kprintf_pos(ROW, COLUMN, "Time since boot: %d s", ++i);
    }
}
