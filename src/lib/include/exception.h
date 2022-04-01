#ifndef __EXCEPTION__H__
#define __EXCEPTION__H__

#include "mini_uart.h"
#include "utils.h"
#include "timer.h"

#define CORE0_INTR_SRC                  ((unsigned int*)(0x40000060))

void no_exception_handle();
void lowerEL_sync_interrupt_handle();
void irq_router();

#endif