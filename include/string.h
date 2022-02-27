#ifndef STRING_H
#define STRING_H

#include "type.h"


int strcmp(char* _str1, char* _str2);
// int streq(char* _str1, char* _str2);

void* memset(void* ptr, int c, size_t size);
void* memcpy(void* dst, void* src, size_t size);


#endif