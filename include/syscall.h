#ifndef SYSCALL_H
#define SYSCALL_H

#include "thread.h"
#include "exception.h"
#include "type.h"

int getpid();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int exec(trapFrame_t *frame, const char *name, char *const argv[]);
int fork(trapFrame_t *frame);
void exit(int status);
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void fork_test();
#endif
