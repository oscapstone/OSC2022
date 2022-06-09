#ifndef SIMPLE_ALLOC_H
#define SIMPLE_ALLOC_H
#include "uart.h"

// from linker.ld
extern char _simple_alloc_start;
extern char _simple_alloc_end;

void* simple_alloc(unsigned int);

#endif