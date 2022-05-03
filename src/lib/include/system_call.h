#ifndef __SYSTEM_CALL__H__
#define __SYSTEM_CALL__H__
#include "schedule.h"
#include "uart.h"
#include "exception.h"
#include "exec.h"
#include "cpio.h"

void sys_get_pid(trapframe_t *tf);
void sys_uart_read(trapframe_t *tf) ;
void sys_uart_write(trapframe_t *tf);
void sys_exec(trapframe_t *tf);
// void sys_fork(trapframe_t *tf);
// void sys_exit(trapframe_t *tf);
// void sys_mbox_call(trapframe_t *tf);
// void sys_kill(trapframe_t *tf);

/* from boot.S */
// void load_reg();

#endif