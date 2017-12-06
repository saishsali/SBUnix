#include <sys/kprintf.h>
#include <sys/process.h>
#include <unistd.h>
#include <sys/syscall.h>
#define ROW 24
#define COLUMN 30
#define FREQUENCY 18

int i = 0, timer = 0;
extern task_struct *process_list_head;

void timer_interrupt() {
    task_struct *pcb = process_list_head;
    timer++;
    if (timer % FREQUENCY == 0) {
        // sys_yield();
        kprintf_pos(ROW, COLUMN, "Time since boot: %d s", ++i);

        while (pcb != NULL) {
            if(pcb->sleep_time > 0)
                pcb->sleep_time--;

            if (pcb->sleep_time == 0 && pcb->state == SLEEPING) {
                pcb->state = READY;
            }
            pcb = pcb->next;
        }
    }
}
