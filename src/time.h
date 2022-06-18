#ifndef __TIME_H__
#define __TIME_H__

#include "stddef.h"
#include "uart.h"
#include "thread.h"

uint64_t start_time;

void set_start_time();
void print_sec();
void set_timer_interrupt (int enable);

#endif