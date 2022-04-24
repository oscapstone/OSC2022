#include "utils.h"

unsigned long align_by_4(unsigned long value) {
    unsigned long lower_bits = value & 0x3;
    value = value - lower_bits;
    if (lower_bits > 0) {
        value += 4;
    }
    return value;
}