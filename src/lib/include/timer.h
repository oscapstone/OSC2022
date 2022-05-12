#ifndef __TIMER__H__
#define __TIMER__H__
#include "stddef.h"
#include "uart.h"
#include "heap.h"

void core_timer_enable ();
unsigned long get_time ();
void set_video_timer();


#endif