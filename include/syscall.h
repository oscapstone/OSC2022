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
    unsigned long sp_el0, spsr_el1, elr_el1;
}trap_frame;

int sys_getpid(trap_frame*);
unsigned int sys_uart_read(trap_frame*,char buf[],unsigned int size);
unsigned int sys_uart_write(trap_frame*,const char* name,unsigned int size);
int sys_exec(trap_frame*,const char* name, char *const argv[]);
int sys_fork(trap_frame*);
void sys_exit(trap_frame*);
int sys_mbox_call(trap_frame*,unsigned char ch, unsigned int *mbox);
void sys_kill(trap_frame*,int pid);

#endif