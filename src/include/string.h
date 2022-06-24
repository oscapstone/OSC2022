#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifndef _DEF_STRING
#define _DEF_STRING

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, int n);
size_t u322hex(uint32_t num, char* buf, size_t len);
size_t u642hex(uint64_t num, char* buf, size_t len);
int atoi(const char* buf);
size_t strlen(const char *str);
void *memcpy(void *dst, const void *src, size_t n);
size_t u642dec(uint64_t num, char* buf, size_t len);
char *strformat(char *buf, uint64_t len, const char *format, ...);
char *vstrformat(char *buf, uint64_t len, const char *format, va_list ap);

#endif