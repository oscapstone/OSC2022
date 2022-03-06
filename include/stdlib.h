#ifndef STDLIB_H
#define STDLIB_H

#ifndef WITH_STDLIB
#include "type.h"

#define _ALIGNSIZE(p)  (p-1)
#define _ALIGN(p, size)  (((uint32_t)(p) +_ALIGNSIZE(size)) & ~_ALIGNSIZE(size))

int atoi(char* str, int base);
#else
#include <stdint.h>
#include <stdlib.h>
#endif











#endif