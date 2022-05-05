#ifndef SYSCALL_H
#define SYSCALL_H
#include "tpf.h"
#include "cpio.h"
#include "mbox.h"
#include "sched.h"
#include "buddy.h"
#include "exception.h"

int getpid(trapframe_t *tpf);
unsigned long uartread(trapframe_t *tpf, char buf[], unsigned long size);
unsigned long uartwrite(trapframe_t *tpf, const char buf[], unsigned long size);
int exec(trapframe_t *tpf, const char *name, char *const argv[]);
int fork(trapframe_t *tpf);
void exit(trapframe_t *tpf, int status);
int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox);
void kill(trapframe_t *tpf, int pid);
void signal_register(int signal, void (*handler)());
void signal_kill(int pid, int signal);
void sigreturn(trapframe_t *tpf);

#endif