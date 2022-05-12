#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include "list.h"

void buddy_system_init();
unsigned long int buddy_system_alloc(int size);
void buddy_system_free(int index);

#endif /* BUDDY_SYSTEM_H */