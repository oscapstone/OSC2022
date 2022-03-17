#ifndef __STRING_H__
#define __STRING_H__

#include "stddef.h"
#include "uart.h"

char *ptr;

void* memcpy( void* dest, const void* src, unsigned long count );
int strncmp( const char *lhs, const char *rhs, unsigned long count );
int strcmp(char *s1, char *s2);
char *strcat( char *dest, const char *src );
char* strtok( char* str, const char delim );
size_t strlen( const char* str ); // return string length without null-terminated byte

#endif