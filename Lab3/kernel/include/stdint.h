#ifndef _STDINT_H
#define _STDINT_H

typedef unsigned long size_t;

typedef unsigned long long int uint64_t;
typedef signed long long int int64_t;

typedef unsigned int uint32_t;
typedef signed int int32_t;

typedef unsigned short int uint16_t;
typedef signed short int int16_t;

typedef unsigned char uint8_t;
typedef signed char int8_t;

//typedef char int8_t;
//typedef short int16_t;
//typedef int int32_t;
//typedef long long int64_t;
//
//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned int uint32_t;
//typedef unsigned long long uint64_t;

uint8_t hex_to_int64(char*);
uint64_t hex_to_int8(char);
#endif