#include <sys/timer.h>
#include <sys/keyboard.h>

// Handler for first 32 interrupts (offset 0 - 31)
void interrupt_handler0() {}

// Handler for timer interrupt (offset 32)
void interrupt_handler32() {
    // timer_interrupt();
}

// Handler for keyboard interrupt (offset 33)
void interrupt_handler33() {
    // keyboard_interrupt();
}
