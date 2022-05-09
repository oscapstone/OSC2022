#ifndef _STRING_H_
#define _STRING_H_

#include "types.h"

char * itoa(int32_t, char*, uint32_t);
char * utoa(uint32_t, char*, uint32_t);
char * ltoa(int64_t, char*, uint32_t);
char * ultoa(uint64_t, char*, uint32_t);
int32_t strcmp(char*, char*);
#endif
