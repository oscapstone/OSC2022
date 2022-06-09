#ifndef _SYSCALL_H_
#define _SYSCALL_H_
#include "types.h"
extern uint64_t sys_hello(uint64_t);
extern size_t sys_uart_write(char *, size_t);
extern size_t sys_uart_read(char *, size_t);
#endif
