#include "utils.h"

unsigned long align_by_4(unsigned long value) {
    unsigned long lower_bits = value & 0x3;
    value = value - lower_bits;
    if (lower_bits > 0) {
        value += 4;
    }
    return value;
}

int log2(int input)
{
	int num = 1;
	int power = 0;

	while (num != input)
	{
		num = num << 1;
		power++;
	}

	return power;
}