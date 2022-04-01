#include <stdint.h>
#include <stddef.h>

#ifndef _DEF_STRING
#define _DEF_STRING

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, int n);
void u322hex(uint32_t num, char* buf, size_t len);
void u642hex(uint32_t num, char* buf, size_t len);
int atoi(const char* buf);
size_t strlen(const char *str);
void *memcpy(void *dst, const void *src, size_t n);

#endif