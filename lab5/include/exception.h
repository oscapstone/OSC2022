#ifndef _EXCEPTION_H
#define _EXCEPTION_H

// save_all, load_all 的 register 用 struct 包起來，方便操作
typedef struct Trapframe {
    unsigned long x[31]; // general register from x0 ~ x30
    unsigned long sp_el0;
    unsigned long elr_el1;
    unsigned long spsr_el1;
} Trapframe;

typedef struct cpio_newc_header {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
} cpio_newc_header;

extern void enable_irq();
extern void disable_irq();
void sync_exception_router(unsigned long esr_el1, unsigned long elr_el1, Trapframe *trapframe);
void syscall(unsigned long syscall_num, Trapframe *trapframe);
void sys_getpid(Trapframe *trapframe);
void sys_uart_read(Trapframe *trapframe);
void sys_uart_write(Trapframe *trapframe);
void sys_exec(Trapframe *trapframe);
void sys_fork(Trapframe *trapframe);
void sys_exit(Trapframe *trapframe);
void sys_mbox_call(Trapframe *trapframe);
void sys_kill(Trapframe *trapframe);

#endif /* _EXCEPTION_H */