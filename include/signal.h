#ifndef _SIGNAL_HEADER_
#define _SIGNAL_HEADER_
#include "syscall.h"

void run_signal(trap_frame*);
void signal_default_handler(int);
void sigreturn();
void signal_registed_handler(void (*handler)());
#endif