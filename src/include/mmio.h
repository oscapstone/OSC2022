#include <stdint.h>
#ifndef _DEF_MMIO
#define _DEF_MMIO

#define MMIO_BASE       0xffff00003f000000
#define MAILBOX_BASE    MMIO_BASE + 0xb880

#define MAILBOX_READ    MAILBOX_BASE
#define MAILBOX_STATUS  MAILBOX_BASE + 0x18
#define MAILBOX_WRITE   MAILBOX_BASE + 0x20

#define PM_RSTC MMIO_BASE + 0x10001c
#define PM_WDOG MMIO_BASE + 0x100024

#define AUX_ENABLES       MMIO_BASE + 0x215004
#define AUX_MU_CNTL_REG   MMIO_BASE + 0x215060
#define AUX_MU_IER_REG    MMIO_BASE + 0x215044
#define AUX_MU_LCR_REG    MMIO_BASE + 0x21504c
#define AUX_MU_MCR_REG    MMIO_BASE + 0x215050
#define AUX_MU_BAUD_REG   MMIO_BASE + 0x215068
#define AUX_MU_IIR_REG    MMIO_BASE + 0x215048
#define AUX_MU_LSR_REG    MMIO_BASE + 0x215054
#define AUX_MU_IO_REG     MMIO_BASE + 0x215040
#define GPFSEL1           MMIO_BASE + 0x200004
#define GPPUD             MMIO_BASE + 0x200094
#define GPPUDCLK0         MMIO_BASE + 0x200098
#define GPPUDCLK1         MMIO_BASE + 0x20009c

//inline void mmio_set(uint32_t addr, unsigned int value);
//inline unsigned int mmio_load(uint32_t addr);

#define mmio_set(ADDR,VAL) *(uint32_t *)(ADDR) = (uint32_t)(VAL)
#define mmio_load(ADDR) *(uint32_t *)(ADDR)

#endif