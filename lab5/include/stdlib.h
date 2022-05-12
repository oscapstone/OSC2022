#ifndef STDLIB_H
#define STDLIB_H

void mem_init();
void *malloc(size_t size);
void free(unsigned long int ptr);
int atoi(const char *str);

#endif /* STDLIB_H */