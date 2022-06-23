#ifndef SYSCALL_H
#define SYSCALL_H
#include "tpf.h"
#include "vfs.h"
#include "cpio.h"
#include "mbox.h"
#include "sched.h"
#include "buddy.h"
#include "exception.h"

#define MAX_FD 16

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

int sys_open(trapframe_t *tpf, const char *pathname, int flags);
int sys_close(trapframe_t *tpf, int fd);
long sys_write(trapframe_t *tpf, int fd, const void *buf, unsigned long count);
long sys_read(trapframe_t *tpf, int fd, void *buf, unsigned long count);
int sys_mkdir(trapframe_t *tpf, const char *pathname, unsigned mode);
int sys_mount(trapframe_t *tpf, const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int sys_chdir(trapframe_t *tpf, const char *path);

#endif