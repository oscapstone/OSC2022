#include "../include/malloc.h"

void* simple_malloc(unsigned int size){
	static void *top = 0;
	void *old = top;
	top += size;
	return old;
}