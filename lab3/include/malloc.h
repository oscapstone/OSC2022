#ifndef MALLOC_H
#define MALLOC_H

// like C malloc
void *simple_malloc(unsigned int size);
void *kmalloc(unsigned int size);
void kfree(void *ptr);

#endif