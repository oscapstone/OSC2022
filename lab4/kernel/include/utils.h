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
uint8_t hex_to_int64(char*);
uint64_t hex_to_int8(char);
uint64_t hex2int(char *hex, int len);
uint64_t align_up(uint64_t addr, uint64_t alignment);
uint64_t align_up_exp(uint64_t n);
#define kb ((uint64_t)0x400)
#define mb ((uint64_t)0x100000)
#define gb ((uint64_t)0x40000000)
uint64_t log2(uint64_t num); 

#endif