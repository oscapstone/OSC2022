
#ifndef __STDLIB_H__
#define __STDLIB_H__

#include "stddef.h"
#include "uart.h"

#define ALIGN(x,a) __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask) (((x)+(mask))&~(mask))

int atoi(const char *s);
void exit();

#endif