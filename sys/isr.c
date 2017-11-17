#include <sys/timer.h>
#include <sys/keyboard.h>
#include <sys/syscall.h>
#include <sys/defs.h>

// Handler for first 32 interrupts (offset 0 - 31)
void interrupt_handler0() {}

// Handler for timer interrupt (offset 32)
void interrupt_handler32() {
    timer_interrupt();
}

// Handler for keyboard interrupt (offset 33)
void interrupt_handler33() {
    keyboard_interrupt();
}

// Handler for syscall
void interrupt_handler128() {
	uint64_t syscall_no;
	__asm__ __volatile__("movq %%rax, %0;" : "=r"(syscall_no));
    syscall_handler(syscall_no);
}
