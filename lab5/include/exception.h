#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "typedef.h"

// save_all, load_all 的 register 用 struct 包起來，方便操作
typedef struct Trapframe {
    uint64_t x[31]; // general register from x0 ~ x30
    uint64_t sp_el0;
    uint64_t elr_el1;
    uint64_t spsr_el1;
} Trapframe;

void sync_exception_router(uint64_t esr_el1, uint64_t elr_el1, Trapframe *trapframe);
void syscall(uint64_t syscall_num, Trapframe* trapframe);
void sys_getpid(Trapframe *trapframe);
void sys_fork(Trapframe *trapframe);
void sys_exit(Trapframe *trapframe);
#endif /* _EXCEPTION_H */