#include "fork.h"
#include "task.h"
#include "syscall.h"
#include "mini_uart.h"
#include "exec.h"
#include "textio.h"
#include "mailbox.h"
#include "list.h"

extern struct taskControlBlock *currentTask;
extern struct taskControlBlock tasks[MAX_TASKS];
extern struct list readylist;
extern struct listItem taskListItem[MAX_TASKS];

void syscall_kill(int pid) {
  if (pid == currentTask->pid) {
    currentTask->state = eTerminated;
    while(1) asm volatile("nop");
  } else {
    // kprintf("You have no permission to kill %d\n", pid);
    kprintf("kill process %d\n", pid);
    disable_preempt();
    tasks[pid].state = eTerminated;
    listRemoveItem(&readylist, &taskListItem[pid]);
  }
}

void *const syscall_vectors[] = {
    syscall_getpid,
    syscall_uart_read,
    syscall_uart_write,
    syscall_exec,
    syscall_fork,
    syscall_exit,
    syscall_mbox_call,
    syscall_kill
};
