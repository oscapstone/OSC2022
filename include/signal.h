#ifndef SIGNAL_H
#define SIGNAL_H
#include "tpf.h"
#include "sched.h"
#include "buddy.h"
#include "syscall.h"
#include "exception.h"

#define USER_SIG_WRAPPER_VIRT_ADDR_ALIGNED 0xffffffff9000L

void check_signal(trapframe_t *tpf);
void run_signal(trapframe_t* tpf, int signal);
void signal_handler_wrapper();
void signal_default_handler();

#endif