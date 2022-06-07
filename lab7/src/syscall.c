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
  if (pid == 0 || pid >= MAX_TASKS) {
    kprintf("Cannot kill task no.%d\n", pid);
  } else if (pid == currentTask->pid) {
    currentTask->state = eTerminated;
    schedule();
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
    syscall_kill,
    NULL,
    NULL,
    NULL,
    syscall_open, // syscall nr 11
    syscall_close,
    syscall_write,
    syscall_read,
    syscall_mkdir,
    syscall_mount,
    syscall_chdir
};
