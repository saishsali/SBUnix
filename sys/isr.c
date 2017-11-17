#include <sys/timer.h>
#include <sys/keyboard.h>
#include <sys/kprintf.h>
#include <sys/isr.h>
#include <sys/page_fault.h>

void interrupt_handler(stack_registers *registers) {
    switch (registers->interrupt_number) {
        case 14:
            page_fault_exception(registers);
            break;
        case 32:
            timer_interrupt();
            break;
        case 33:
            keyboard_interrupt();
            break;
    }
}
