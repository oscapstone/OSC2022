#ifndef	SYSCALL_H_
#define	SYSCALL_H_

typedef struct _TrapFrame{
    unsigned long long x[31];
    unsigned long long sp_el0;
    unsigned long long elr_el1;
    unsigned long long spsr_el1;
}TrapFrame;


void sys_getpid(TrapFrame *);
void sys_uart_read(TrapFrame *);
void sys_uart_write(TrapFrame *);
void sys_exec(TrapFrame *);
void sys_fork(TrapFrame *);
void sys_exit(TrapFrame *);
void sys_mbox_call(TrapFrame *);
void sys_kill(TrapFrame *);

int do_getpid();
void do_fork();
void do_exit();
void do_kill();

#endif 