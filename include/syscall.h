#ifndef SYSCALL_H
#define SYSCALL_H

#include "thread.h"
#include "exception.h"
#include "type.h"
#include "dev_framebuffer.h"

int getpid();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int exec(trapFrame_t *frame, char *name, char *const argv[]);
int fork(trapFrame_t *frame);
void exit(int status);
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void fork_test();
uint64 sys_mmap(void *addr, size_t len, int prot, int flags, int fd, int file_offset);

int call_vfs_open(const char *pathname, int flags);
int call_vfs_close(int fd);
long call_vfs_write(int fd, const char *buf, unsigned long count);
long call_vfs_read(int fd, char *buf, unsigned long count);
int call_vfs_mkdir(const char *pathname, unsigned mode);
int call_vfs_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int call_vfs_chdir(const char *path);
long call_vfs_lseek64(int fd, long offset, int whence);
int call_vfs_ioctl(int fd, unsigned long request, framebuffer_info_t *fb_info);
#endif
