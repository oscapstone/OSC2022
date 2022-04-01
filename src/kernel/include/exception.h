#ifndef _DEF_EXCEPTION
#define _DEF_EXCEPTION

#include <stdint.h>

extern uint64_t exception_vector_table;

void exception_vector_table_init();
// void exception_handler();

#endif