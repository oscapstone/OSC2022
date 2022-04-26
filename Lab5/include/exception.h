#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "task.h"
#include <stddef.h>

void enable_interrupt();
void disable_interrupt();
void dumpState();
void lower_sync_handler();
void lower_iqr_handler();
void curr_sync_handler();
void curr_iqr_handler();
void error_handler();
void child_return_from_fork();

/* Implement system calls */
int sys_getpid();
size_t sys_uartread(char buf[], size_t size);
size_t sys_uartwrite(const char buf[], size_t size);
int sys_exec(trap_frame *tf, const char *name, char *const argv[]);
void sys_fork(trap_frame *tf);
void sys_exit();
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);

#endif
