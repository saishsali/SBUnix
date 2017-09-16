#include <sys/kprintf.h>
int i = 1;

void timer_interrupt() {
    i++;
    kprintf("%d\n", i);
    kprintf("Hello");
}
