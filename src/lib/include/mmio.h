#ifndef __MMIO__H__
#define __MMIO__H__
#define PHY_BASE_ADDRESS 0x3F000000
#define BUS_BASE_ADDRESS 0x7E000000

#if USE_BUS_ADDRESS
#define MMIO_BASE_ADDRESS BUS_BASE_ADDRESS
#else
#define MMIO_BASE_ADDRESS PHY_BASE_ADDRESS
#endif

void mmio_put(long addr, unsigned int value);
unsigned int mmio_get(long addr);
#endif