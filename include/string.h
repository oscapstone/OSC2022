#ifndef STRING_H
#define STRING_H

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#include <stdlib.h>
#endif

int strcmp(char* _str1, char* _str2);
int memcmp(void* _src1, void* _src2, size_t n);
void* memset(void* ptr, int c, size_t size);
void* memcpy(void* dst, void* src, size_t size);



uint32_t _strlen(char* str);



#endif