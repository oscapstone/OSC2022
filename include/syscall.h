#ifndef _SYSCALL_HEADER_
#define _SYSCALL_HEADER_

#define SYS_GETPID          0
#define SYS_UART_READ       1
#define SYS_UART_WRITE      2
#define SYS_EXEC            3
#define SYS_FORK            4
#define SYS_EXIT            5
#define SYS_MBOX_CALL       6
#define SYS_KILL            7

typedef struct trap_frame{
    unsigned long long x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10,
                       x11, x12, x13, x14, x15, x16, x17, x18, x19, x20,
                       x21, x22, x23, x24, x25, x26, x27, x28, x29, x30;
    unsigned long long sp_el0, spsr_el1, elr_el1;
}trap_frame;

int sys_getpid(trap_frame*);
unsigned int sys_uart_read(trap_frame*,char buf[],unsigned long long size);
unsigned int sys_uart_write(trap_frame*,const char* name,unsigned long long size);
int sys_exec(trap_frame*,const char* name, char *const argv[]);
int sys_fork(trap_frame*);
void sys_exit(trap_frame*);
int sys_mbox_call(trap_frame*,unsigned char ch, unsigned int *mbox);
void sys_kill(trap_frame*,int pid);
void signal_register(int signal,void (*handler)());
void signal_kill(int pid,int signal);


void sys_ls(trap_frame* tf);
// syscall number : 11
int sys_open(trap_frame* tf,const char *pathname, int flags);
// syscall number : 12
int sys_close(trap_frame* tf,int fd);
// syscall number : 13
// remember to return read size or error code
long sys_write(trap_frame* tf,int fd, const void *buf, unsigned long count);
// syscall number : 14
// remember to return read size or error code
long sys_read(trap_frame* tf,int fd, void *buf, unsigned long count);
// syscall number : 15
// you can ignore mode, since there is no access control
int sys_mkdir(trap_frame* tf,const char *pathname, unsigned mode);
// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int sys_mount(trap_frame* tf,const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
// syscall number : 17
int sys_chdir(trap_frame* tf,const char *path);

#endif