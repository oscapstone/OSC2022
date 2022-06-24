#include "math.h"

int pow(int base, int exp)
{
    int value = 1;
    for (int i = 0; i < exp; i++)
        value *= base;

    return value;
}

int log2_ceiling(uint32_t num)
{
    int value = -1;
    uint32_t tmp = num;
    while(tmp > 0) {
        tmp >>= 1;
        value++;
    }
    value += ((num - (1 << value)) != 0);

    return value;
}

int log2_floor(uint32_t num)
{
    int value = -1;
    while(num > 0) {
        num >>= 1;
        value++;
    }

    return value;
}