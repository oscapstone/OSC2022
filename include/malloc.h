#ifndef MALLOC_H
#define MALLOC_H
#include "uart.h"
#include "list.h"
#include "buddy.h"
#include "simple_alloc.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define ALIGN(num, base) ((num + base - 1) & ~(base - 1))

unsigned int find_size_idx(int size);
void sc_alloc_init();
void sc_init();

void *sc_alloc(int size);
int sc_free(void *sc);

void *kmalloc(int size);
void kfree(void *ptr);

void sc_test();

#endif