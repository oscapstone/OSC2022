#ifndef _GPIO_H
#define _GPIO_H
#define MMIO_BASE       0x3f000000

// BCM2837 peripheral: https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf#page=8
// AUX UART
#define AUX_ENABLE      ((volatile uint*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile uint*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile uint*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile uint*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile uint*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile uint*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile uint*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile uint*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile uint*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile uint*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile uint*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile uint*)(MMIO_BASE+0x00215068))

#endif