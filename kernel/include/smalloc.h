#ifndef __SMALLOC_H__
#define __SMALLOC_H__

#define NULL (void *)0

extern unsigned long __heap_start;
extern unsigned long __heap_size;
extern unsigned long __heap_end;

static unsigned long heap_start = (unsigned long) &__heap_start;
static unsigned long heap_size = (unsigned long) &__heap_size;
static unsigned long heap_end = (unsigned long) &__heap_end;

void *smalloc(unsigned long size);

#endif