#include "utils.h"
#define MMIO_BASE   0x3F000000

#define AUXENB      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO_REG       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER_REG     ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR_REG      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR_REG      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR_REG      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR_REG      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR_REG       ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH_REG   ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL_REG     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT_REG      ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD_REG      ((volatile unsigned int*)(MMIO_BASE+0x00215068))
void init_uart();
char read_uart();
int read_int();
void writec_uart(unsigned int s);
void writes_uart(char *s);
void writes_n_uart(char *s, unsigned int size);
void writes_nl_uart(char *s);
void writehex_uart(unsigned int h,int newline);
void write_int_uart(unsigned int,bool newline);
void busy_wait_writehex(unsigned long long h,bool newline);
//void writeaddr_uart(unsigned int*);
#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

#define IRQ_ENABLE1 ((volatile unsigned *)(0x3F00B210))
char uart_buf_read[1024];
char uart_buf_write[1024];
int uart_read_i_l;
int uart_read_i_r;
int uart_write_i_l;
int uart_write_i_r;
void init_uart_buf();
void uart_buf_read_push(char c);
void uart_buf_write_push(char c);
void uart_buf_writes_push(char *s);
char uart_buf_read_pop();
char uart_buf_write_pop();
void busy_wait_writes(char *s,bool newline);
void busy_wait_writec(char s);
void busy_wait_writeint(int i,bool newline);
int is_empty_write();
int is_empty_read();

#ifdef __DEBUG_LOG
    static inline void writes_uart_debug(char* s, bool newline){
         while(*s){
            if(*s=='\n')
                writec_uart('\r');
            writec_uart(*s++);
        }
        if(newline)
            writes_uart("\r\n");
    }
#endif