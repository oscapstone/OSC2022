#ifndef _USER_LIB_H_
#define _USER_LIB_H_
#include "types.h"
#include <stdarg.h>

extern size_t uart_write(char *, size_t);
extern size_t uart_read(char *, size_t);
extern uint64_t fork(void);
extern uint64_t getpid(void);
extern void debug_info(void);
extern void delay(uint64_t);
extern int32_t printf(char *, ...);

#endif
