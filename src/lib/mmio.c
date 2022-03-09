#include <mmio.h>
#include <stdint.h>

/*
inline void mmio_set(uint32_t addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

inline unsigned int mmio_load(uint32_t addr) {
    volatile unsigned int* point = (unsigned int*)addr;
    return *point;
}*/