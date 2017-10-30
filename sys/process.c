#include <sys/process.h>
#include <sys/memory.h>

int get_process_id() {
    int i;
    for (i = 0; i < MAX_PROCESS; i++) {
        if (process_ids[i] == 0) {
            process_ids[i] = 1;
            return i;
        }
    }
    return -1;
}

void create_process() {
    task_struct *process = kmalloc(sizeof(task_struct));
    process->state = RUNNING;
    process->pid = get_process_id();

}
