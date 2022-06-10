
#ifndef __STDLIB_H__
#define __STDLIB_H__

#include "stddef.h"
#include "uart.h"

#define ALIGN(x,a) __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask) (((x)+(mask))&~(mask))

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })

static uint32_t heap_start  = 0x10000000;
static uint32_t heap_size = 0x10000000;
static uint32_t cur_heap_pos = 0x10000000;

int atoi(const char *s);
void exit();

#endif