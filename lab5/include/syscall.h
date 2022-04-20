#ifndef SYSCALL_H
#define SYSCALL_H

#include "stddef.h"

int getpid();
size_t uartread(char buf[], size_t size);
size_t uartwrite(const char buf[], size_t size);
int exec(const char *name, char *const argv[]);
int fork();
void exit(int status);
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);

#endif
