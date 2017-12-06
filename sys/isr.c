#include <sys/timer.h>
#include <sys/keyboard.h>
#include <sys/kprintf.h>
#include <sys/isr.h>
#include <sys/page_fault.h>
#include <sys/syscall.h>

void interrupt_handler(stack_registers *registers) {
    switch (registers->interrupt_number) {
        case 0:
            kprintf("Divide by zero exception\n");
            // sys_exit();
            while(1);
            break;
        case 14:
            page_fault_exception(registers);
            break;
        case 32:
            timer_interrupt();
            break;
        case 33:
            keyboard_interrupt();
            break;
        case 128:
            syscall_handler(registers);
            break;
    }
}
