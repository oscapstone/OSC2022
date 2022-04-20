#ifndef _EXCEPTION_H
#define _EXCEPTION_H

void enable_interrupt();
void disable_interrupt();
void dumpState();
void lower_sync_handler();
void lower_iqr_handler();
void curr_sync_handler();
void curr_iqr_handler();
void error_handler();

#endif
