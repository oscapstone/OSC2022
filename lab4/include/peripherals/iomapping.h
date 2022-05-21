#ifndef _IOMAPPING_H_
#define _IOMAPPING_H_


/* Physical addresses range from 0x3F000000 to 0x3FFFFFFF for peripherals */ 
#define IO_BASE 0x3F000000

/* Register's offset of GPIO */
#define GPFSEL0 (IO_BASE + 0x200000)
#define GPFSEL0 (IO_BASE + 0x200000)
#define GPFSEL1 (IO_BASE + 0x200004)
#define GPFSEL2 (IO_BASE + 0x200008)
#define GPFSEL3 (IO_BASE + 0x20000C)
#define GPFSEL4 (IO_BASE + 0x200010)
#define GPFSEL5 (IO_BASE + 0x200014)
#define GPSET0 (IO_BASE + 0x20001C)
#define GPSET1 (IO_BASE + 0x200020)
#define GPCLR0 (IO_BASE + 0x200028)
#define GPCLR1 (IO_BASE + 0x20002C)
#define GPLEV0 (IO_BASE + 0x200034)
#define GPLEV1 (IO_BASE + 0x200038)
#define GPEDS0 (IO_BASE + 0x200040)
#define GPEDS1 (IO_BASE + 0x200044)
#define GPREN0 (IO_BASE + 0x20004C)
#define GPREN1 (IO_BASE + 0x200050)
#define GPFEN0 (IO_BASE + 0x200058)
#define GPFEN1 (IO_BASE + 0x20005C)
#define GPHEN0 (IO_BASE + 0x200064)
#define GPHEN1 (IO_BASE + 0x200068)
#define GPLEN0 (IO_BASE + 0x200070)
#define GPLEN1 (IO_BASE + 0x200074)
#define GPAREN0 (IO_BASE + 0x20007C)
#define GPAREN1 (IO_BASE + 0x200080)
#define GPAFEN0 (IO_BASE + 0x200088)
#define GPAFEN1 (IO_BASE + 0x20008C)
#define GPPUD (IO_BASE + 0x200094)
#define GPPUDCLK0 (IO_BASE + 0x200098)
#define GPPUDCLK1 (IO_BASE + 0x20009C)


/* Register's offset of UART and SPI */
#define AUX_IRQ (IO_BASE + 0x215000)
#define AUX_ENABLES (IO_BASE + 0x215004)
#define AUX_MU_IO_REG (IO_BASE + 0x215040)
#define AUX_MU_IER_REG (IO_BASE + 0x215044)
#define AUX_MU_IIR_REG (IO_BASE + 0x215048)
#define AUX_MU_LCR_REG (IO_BASE + 0x21504C)
#define AUX_MU_MCR_REG (IO_BASE + 0x215050)
#define AUX_MU_LSR_REG (IO_BASE + 0x215054)
#define AUX_MU_MSR_REG (IO_BASE + 0x215058)
#define AUX_MU_SCRATCH (IO_BASE + 0x21505C)
#define AUX_MU_CNTL_REG (IO_BASE + 0x215060)
#define AUX_MU_STAT_REG (IO_BASE + 0x215064)
#define AUX_MU_BAUD_REG (IO_BASE + 0x215068)
#define AUX_SPI0_CNTL0_REG (IO_BASE + 0x215080)
#define AUX_SPI0_CNTL1_REG (IO_BASE + 0x215084)
#define AUX_SPI0_STAT_REG (IO_BASE + 0x215088)
#define AUX_SPI0_IO_REG (IO_BASE + 0x215090)
#define AUX_SPI0_PEEK_REG (IO_BASE + 0x215094)
#define AUX_SPI1_CNTL0_REG (IO_BASE + 0x2150C0)
#define AUX_SPI1_CNTL1_REG (IO_BASE + 0x2150C4)
#define AUX_SPI1_STAT_REG (IO_BASE + 0x2150C8)
#define AUX_SPI1_IO_REG (IO_BASE + 0x2150D0)
#define AUX_SPI1_PEEK_REG (IO_BASE + 0x2150D4)

// mailbox register base address
#define MBOX_REG (IO_BASE + 0xB880)

// interrupt
#define IRQ_BASIC_PENDING (IO_BASE + 0xB200)
#define IRQ_PENDING_1 (IO_BASE + 0xB204)
#define IRQ_PENDING_2 (IO_BASE + 0xB208)
#define CORE0_INTERRUPT_SOURCE (IO_BASE + 0x1000060)
#define ENABLE_IRQS_1 (IO_BASE + 0xB210)


// timer 
#define CORE0_TIMER_IRQ_CTRL   (IO_BASE + 0x1000040)


// watch dog
#define PM_PASSWORD (IO_BASE + 0x1b000000)
#define PM_RSTC (IO_BASE + 0x10001c)
#define PM_WDOG (IO_BASE + 0x100024)

#define IO_MMIO_write32(addr, val) *(uint32_t*)addr = val
#define IO_MMIO_read32(addr) *(uint32_t*)addr 
#define IO_MMIO_write64(addr, val) *(uint64_t*)addr = val
#define IO_MMIO_read64(addr) *(uint64_t*)addr 



#endif
    
