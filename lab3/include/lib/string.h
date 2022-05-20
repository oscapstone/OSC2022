#ifndef _STRING_H_
#define _STRING_H_

#include "types.h"

extern char * itoa(int32_t, char*, uint32_t);
extern char * utoa(uint32_t, char*, uint32_t);
extern char * ltoa(int64_t, char*, uint32_t);
extern char * ultoa(uint64_t, char*, uint32_t);
extern int32_t strcmp(char*, char*);
extern char *strcpy(char *, const char *);
extern size_t strlen(const char*);
extern void* memcpy(void*, const void*, size_t);
extern void* memset(void*, int, size_t);
extern int32_t memcmp(void*, const void*, size_t);
extern uint8_t hex2dec(char);
extern char* strtok(char*,const char*);
#endif
