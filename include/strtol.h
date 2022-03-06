#ifndef STRTOL_H
#define STRTOL_H

#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif


int32_t _strtol(const char* str, char** endptr, int base);

uint32_t _strtoul(const char* str, char** endptr, int base);

int64_t _strtoll(const char* str, char** endptr, int base);

uint64_t _strtoull(const char* str, char** endptr, int base);


#endif