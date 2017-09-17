#include <sys/timer.h>
#include <sys/kprintf.h>

void interrupt_handler0() {}

void interrupt_handler32() {
    timer_interrupt();
}

void interrupt_handler33() {}
