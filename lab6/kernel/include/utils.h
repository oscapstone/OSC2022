#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdint.h>
uint64_t hex_to_int64(char* num);
uint8_t hex_to_int8(char hex);
uint64_t hex2int(char *hex, int len);
uint64_t align_up(uint64_t addr, uint64_t alignment);
uint64_t align_up_exp(uint64_t n);
#define kb ((uint64_t)0x400)
#define mb ((uint64_t)0x100000)
#define gb ((uint64_t)0x40000000)
uint64_t log2(uint64_t num); 
void delay(int num);
uint32_t be2le(uint32_t x);
#endif
