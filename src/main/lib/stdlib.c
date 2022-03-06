
#ifndef WITH_STDLIB
#include "type.h"
#include "strtol.h"
#include "stdlib.h"

int atoi(char* str, int base) {
    return (int)_strtol(str, NULL, base);
}


#endif



