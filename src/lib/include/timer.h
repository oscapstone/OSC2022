#ifndef __TIMER__H__
#define __TIMER__H__

#define CORE0_TIMER_IRQ_CTRL            ((unsigned int*)(0x40000040))

#include "exception.h"
#include "mini_uart.h"

void core_timer_enable();
void core_timer_interrupt();

#endif