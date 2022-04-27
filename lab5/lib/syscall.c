#include "syscall.h"
#include "mini_uart.h"

int sys_getpid() {

}

unsigned sys_uartread(char buf[], unsigned size) {

}

unsigned sys_uartwrite(const char buf[], unsigned size) {

} // check

int sys_exec(const char *name, char *const argv[]) {

}

int sys_fork() {

} //check

void sys_exit(int status) {

} // check

int sys_mbox_call(unsigned char ch, unsigned int *mbox) {

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