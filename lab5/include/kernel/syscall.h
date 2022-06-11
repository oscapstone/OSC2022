#ifndef _SYSCALL_H_
#define _SYSCALL_H_
#include "types.h"
extern uint64_t sys_hello(uint64_t);
extern size_t sys_uart_write(char *, size_t);
extern size_t sys_uart_read(char *, size_t);
extern uint64_t sys_fork();
extern uint64_t sys_getpid();
extern int sys_mbox_call(uint8_t, uint32_t *);
extern void sys_exit();
extern void sys_kill(uint64_t pid);
extern int sys_exec(const char *, char *const []);
#endif
