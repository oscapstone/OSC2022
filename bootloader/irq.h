#ifndef IRQ_H
#define IRQ_H

void handle_el1_irq();
void handle_el0_irq();
void exception_entry();
void enable_interrupt();
void disable_interrupt();
#endif
