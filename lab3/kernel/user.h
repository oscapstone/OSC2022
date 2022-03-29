#ifndef USER_H
#define USER_H

#include "stdint.h"

extern void branch_to_address_in_el0(uint64_t instr_addr, uint64_t stack_addr);
extern void set_exception_vector_table();
extern void core_timer_enable();
extern void reset_timer(uint32_t time_in_cycle);

#endif
