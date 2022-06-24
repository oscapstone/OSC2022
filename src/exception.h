#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "thread.h"
#include "uart.h"
#include "mbox.h"
#include "cpio.h"
#include "mmu.h"

typedef struct trap_frame {
    unsigned long regs[31];
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long sp_el0;
} trap_frame_t;

void svc_handler(trap_frame_t* trap_frame);
int exec(const char *name, char *const argv[], trap_frame_t* trap_frame);
void set_interrupt(int enable);

#endif
