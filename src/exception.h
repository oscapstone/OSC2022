#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "thread.h"
#include "uart.h"
#include "mbox.h"
#include "cpio.h"

typedef struct trap_frame {
    unsigned long regs[31];
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long sp_el0;
} trap_frame_t;

#endif
