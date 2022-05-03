#include "printf.h"
#include "sched.h"

void end_of_thread() {
    int pid = current_thread()->pid;
    task[pid]->state = ZOMBIE;
    while (1) {
        schedule();
    }
}