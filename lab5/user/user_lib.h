#ifndef _USER_LIB_H_
#define _USER_LIB_H_
#include "types.h"

extern size_t uart_write(char *, size_t);
extern size_t uart_read(char *, size_t);
extern uint64_t fork(void);
extern void delay(uint64_t);

#endif
