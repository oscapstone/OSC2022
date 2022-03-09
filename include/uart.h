#ifndef UART_H
#define UART_H


#include "bcm2357.h"
#include "type.h"


#define _AUX_IRQ_OFF            0x00215000
#define _AUX_ENAB_OFF           0x00215004
#define _AUX_MU_IO_OFF          0x00215040
#define _AUX_MU_IER_OFF         0x00215044
#define _AUX_MU_IIR_OFF         0x00215048
#define _AUX_MU_LCR_OFF         0x0021504c
#define _AUX_MU_MCR_OFF         0x00215050
#define _AUX_MU_LSR_OFF         0x00215054
#define _AUX_MU_MSR_OFF         0x00215058
#define _AUX_MU_SCRATCH_OFF     0x0021505c
#define _AUX_MU_CNTL_OFF        0x00215060
#define _AUX_MU_STAT_OFF        0x00215064
#define _AUX_MU_BAUD_OFF        0x00215068
#define _AUX_SPI0_CNTL0_OFF     0x00215080
#define _AUX_SPI0_CNTL1_OFF     0x00215084
#define _AUX_SPI0_STAT_OFF      0x00215088
#define _AUX_SPI0_IO_OFF        0x00215090
#define _AUX_SPI0_PEEK_OFF      0x00215094
#define _AUX_SPI1_CNTL0_OFF     0x002150c0
#define _AUX_SPI1_CNTL1_OFF     0x002150c4

#define AUX_IRQ            _virt_addr(_AUX_IRQ_OFF          )
#define AUX_ENAB           _virt_addr(_AUX_ENAB_OFF         )
#define AUX_MU_IO          _virt_addr(_AUX_MU_IO_OFF        )
#define AUX_MU_IER         _virt_addr(_AUX_MU_IER_OFF       )
#define AUX_MU_IIR         _virt_addr(_AUX_MU_IIR_OFF       )
#define AUX_MU_LCR         _virt_addr(_AUX_MU_LCR_OFF       )
#define AUX_MU_MCR         _virt_addr(_AUX_MU_MCR_OFF       )
#define AUX_MU_LSR         _virt_addr(_AUX_MU_LSR_OFF       )
#define AUX_MU_MSR         _virt_addr(_AUX_MU_MSR_OFF       )
#define AUX_MU_SCRATCH     _virt_addr(_AUX_MU_SCRATCH_OFF   )
#define AUX_MU_CNTL        _virt_addr(_AUX_MU_CNTL_OFF      )
#define AUX_MU_STAT        _virt_addr(_AUX_MU_STAT_OFF      )
#define AUX_MU_BAUD        _virt_addr(_AUX_MU_BAUD_OFF      )
#define AUX_SPI0_CNTL0     _virt_addr(_AUX_SPI0_CNTL0_OFF   )
#define AUX_SPI0_CNTL1     _virt_addr(_AUX_SPI0_CNTL1_OFF   )
#define AUX_SPI0_STAT      _virt_addr(_AUX_SPI0_STAT_OFF    )
#define AUX_SPI0_IO        _virt_addr(_AUX_SPI0_IO_OFF      )
#define AUX_SPI0_PEEK      _virt_addr(_AUX_SPI0_PEEK_OFF    )
#define AUX_SPI1_CNTL0     _virt_addr(_AUX_SPI1_CNTL0_OFF   )
#define AUX_SPI1_CNTL1     _virt_addr(_AUX_SPI1_CNTL1_OFF   )



void init_uart();
void uart_send(char);
char uart_recv();
void uart_write(char*);

void uart_hex(uint32_t);

char hex2a(uint8_t);



#endif