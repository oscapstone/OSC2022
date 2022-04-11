#ifndef _EXCEPTION_H
#define _EXCEPTION_H

void enable_interrupt();
void disable_interrupt();
void exception_entry ();
void irq_exc_router ();

#endif