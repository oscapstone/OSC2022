#ifndef EXCEPT_H
#define EXCEPT_H

#define CORE0_IRQ_SOURCE ((volatile unsigned int *)(0x40000060))
#define GPU_PENDING1 ((volatile unsigned int *)(0x3F00B204))

void set_exception_vector_table();
void print_exception_info();
void c_irq_handler();
void c_invalid_exception();

void enable_irq();
void disable_irq();

#endif
