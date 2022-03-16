#ifndef UTILS
#define UTILS

#define SWAP32(x) ((x >> 24) | ((x & 0x00FF0000) >> 8) | \
                   (x << 24) | ((x & 0x0000FF00) << 8)) // for big endian
#define SWAP64(x) ((x >> 56) | ((x & 0x00FF000000000000ULL) >> 40) | ((x & 0x0000FF0000000000ULL) >> 24) | ((x & 0x000000FF00000000ULL) >> 8) | \
                   (x << 56) | ((x & 0x000000000000FF00ULL) << 40) | ((x & 0x0000000000FF0000ULL) << 24) | ((x & 0x00000000FF000000ULL) << 8)) // for big endian


void delay_cycle(unsigned int n);
void delay_ms(unsigned int n);
unsigned int str2num(char* str, int len);

#endif