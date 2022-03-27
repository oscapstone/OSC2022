#ifndef _UTILS_H_
#define _UTILS_H_
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
void delay(unsigned long);
void put32(unsigned long, unsigned int);
unsigned int get32(unsigned long);

#endif