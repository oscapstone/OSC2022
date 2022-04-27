#include "syscall.h"
#include "../include/sched.h"
#include "peripherals/mailbox.h"
#include "fork.h"
#include "mm.h"
#include "mini_uart.h"

int sys_getpid() {
    return current->id;
}

unsigned sys_uartread(char buf[], unsigned size) {
    for(int i=0; i<size; i++) {
        buf[i] = uart_recv();
    }
    return size;
}

unsigned sys_uartwrite(const char buf[], unsigned size) {
    printf("%s", buf);
    return size;
} 

int sys_exec(const char *name, char *const argv[]) {

    preempt_disable();

    preempt_enable();

    return -1; // only on failure
}

int sys_fork() {
    return copy_process(0, 0, 0, (unsigned long)malloc(4096));
}

void sys_exit(int status) {
    exit_process();
}

int sys_mbox_call(unsigned char ch, unsigned int *mbox) {
    unsigned int r = (((unsigned int)((unsigned long)mbox)&~0xF) | (ch&0xF));
    while(*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRITE = r;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {}
        if (r == *MAILBOX_READ) {
            return mbox[1]==REQUEST_SUCCEED;
        }
    }
    return 0;
}

void sys_kill(int pid) {

}

void * const sys_call_table[] =
{
    sys_getpid,
    sys_uartread,
    sys_uartwrite,
    sys_exec,
    sys_fork,
    sys_exit,
    sys_mbox_call,
    sys_kill
};