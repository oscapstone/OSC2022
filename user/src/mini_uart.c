#include "mini_uart.h"
#include "stdlib.h"
#include "exception.h"
#include "string.h"
void init_uart(){
    *AUXENB |=1; // enable mini UART, then mini uart register can be accessed.
    *AUX_MU_CNTL_REG = 0; // Disable transmitter and receiver during configuration.
    *AUX_MU_IER_REG = 0; // 0: Disable interrupt, 1: enable interrupt
    *AUX_MU_LCR_REG = 3; // set data size to 8 bits.
    *AUX_MU_MCR_REG = 0; // no flow control
    *AUX_MU_BAUD_REG  = 270; // 250MHz / (115200 + 1)*8
    *AUX_MU_IIR_REG = 6; // No FIFO
    *AUX_MU_CNTL_REG = 3; // Enable the transmitter and receiver.

    

    /* configure gpio*/
    
    register unsigned int tmp_reg; // hope the register can be stored in CPU register.
    // GPFSELn: define the operation of the GPIO pins
    // GPFSEL1: FSEL10(0~2) ~ FSEL19(27~29) + 30,31 reserved
    /*  FSEL: 3bits, in GPFSELn

        000 = GPIO Pin 9 is an input
        001 = GPIO Pin 9 is an output
        100 = GPIO Pin 9 takes alternate function 0
        101 = GPIO Pin 9 takes alternate function 1
        110 = GPIO Pin 9 takes alternate function 2
        111 = GPIO Pin 9 takes alternate function 3
        011 = GPIO Pin 9 takes alternate function 4
        010 = GPIO Pin 9 takes alternate function 5
    */
    tmp_reg = *GPFSEL1; // load the GPIO Function Select Registers SEL1 to tmp_reg.
    tmp_reg &= 0xfffc0fff; // use a mask to clear the 12~17 bits(FSEL14 & FSEL15) of the register.
    // FSEL14: 12~14bits.
    // FSEL15: 15~17bits.
    tmp_reg |= (0b010 << 12) | (0b010 << 15); // to make the 12~14bits and 15~17 bits of the register to b010, takes alternate function 5.
    *GPFSEL1 = tmp_reg;
    /*  
    1. Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither
    to remove the current Pull-up/down)
    2. Wait 150 cycles – this provides the required set-up time for the control signal
    3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
    modify – NOTE only the pads which receive a clock will be modified, all others will
    retain their previous state.
    4. Wait 150 cycles – this provides the required hold time for the control signal
    5. Write to GPPUD to remove the control signal
    6. Write to GPPUDCLK0/1 to remove the clock
    */
    *GPPUD = 0; // disable pull-up/down tmp_regregister, allow gpio pins use alternate function, not only basic input-output.
    tmp_reg = 150;
    while(tmp_reg--){
        asm volatile("nop"); // wait 150 cycles.
    }
    *GPPUDCLK0 = (1<<14)|(1<<15); // assert clock on 14 and 15, because the gpio state would be modified.
    tmp_reg = 150;
    while(tmp_reg--){
        asm volatile("nop"); // wait 150 cycles.
    }

    *GPPUDCLK0 = 0; // remove the clock.
    *AUX_MU_CNTL_REG = 3; // enable tx and rx after configuration.

}
char read_uart(){
    char r;
    while(!(*AUX_MU_LSR_REG & 0x01)){
        asm volatile("nop");
    }
    r = (char)(*AUX_MU_IO_REG);
    return r=='\r'?'\n':r;
}

void write_int_uart(unsigned int s, bool newline){
    if(s==0){
        writec_uart('0');
        if(newline)
            writes_uart("\r\n");
        return;
    }
    char a[128];
    int i=0,n=s;
    while(n!=0){
        a[i] = '0' + n%10;
        n/=10;
        i++;
    }
    a[i]='\0';
    for (int j = i-1; j>=0; j--)
    {
        writec_uart(a[j]);
        /* code */
    }
    
    if(newline)
        writes_uart("\r\n");
}

void writec_uart(unsigned int s){
    // unsigned int c = s;
    // while(!(*AUX_MU_LSR_REG & 0x20)) asm volatile("nop");
    // *AUX_MU_IO_REG = c;
    uart_buf_write_push(s);
}

void writes_n_uart(char *s, unsigned int size){
    for(int i=0;i<size;i++){
        if(*s=='\n')
            writec_uart('\r');
        writec_uart(*s++);
    }
}

void writes_nl_uart(char *s){
    while(*s){
        if(*s=='\n')
            writec_uart('\r');
        writec_uart(*s++);
    }
    writes_uart("\r\n");
}

void writes_uart(char *s){
    // while(*s){
    //     if(*s=='\n')
    //         writec_uart('\r');
    //     writec_uart(*s++);
    // }
    
    // disable_interrupt();
    for (int i = 0; i < strlen(s); i++)
    {
        /* code */
        if(s[i]=='\n')
            uart_buf_write_push('\r');
        uart_buf_write_push(s[i]);
    }
    // enable_interrupt();
}
void writehex_uart(unsigned int h,int newline){
    writes_uart("0x");
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        
        n=(h>>c)&(0xF); // n = 1,2,3....th byte of h from left to right.
        
        n+=n>9?0x37:0x30; // int 0~9 -> char '0'~'9', 10~15 -> 'A'~'F'
        writec_uart(n);
    }
    if(newline==1){
        writes_uart("\r\n");
    }
}
void writeint_uart(unsigned int i){
    int iter=0;
    char buffer[100];
    int k=i;
    while(1){
        if(k==0)
        {
            buffer[iter]=k%10;
            break;
        }else{
            buffer[iter++] = k%10;
            k = k/10;
        }   
    }
    writes_uart(buffer);
    writes_uart("\r\n");
}
// void writeaddr_uart(unsigned int* addrh){

//     writes_uart("0x");
//     unsigned int n;
//     int c;
//     for(c=28;c>=0;c-=4) {
        
//         n=(h>>c)&(0xF); // n = 1,2,3....th byte of h from left to right.
        
//         n+=n>9?0x37:0x30; // int 0~9 -> char '0'~'9', 10~15 -> 'A'~'F'
//         writec_uart(n);
//     }
// }
void init_uart_buf(){
    uart_read_i_l=0;
    uart_read_i_r=0;
    uart_write_i_l=0;
    uart_write_i_r=0;
    //uart_buf_read = simple_malloc(2560);
    uart_buf_read[0]='\0';
    // uart_buf_write = simple_malloc(2560);
    uart_buf_write[0] = '\0';
}
void uart_buf_read_push(char c){
    disable_interrupt();
    if(uart_read_i_r<1024)
    {
        uart_buf_read[uart_read_i_r++] = c;
        uart_read_i_r= uart_read_i_r % 1024;
        // uart_buf_read[uart_read_i_r] = '\0';
    }
    enable_interrupt();
}
void uart_buf_write_push(char c){
    disable_interrupt();
    if(uart_write_i_r<1024)
    {
        uart_buf_write[uart_write_i_r++] = c;
        uart_write_i_r = uart_write_i_r % 1024;
        // uart_buf_write[uart_write_i_r] = '\0';
        
        // if(*AUX_MU_IER_REG != 3)
        //     *AUX_MU_IER_REG = 3;
        if(!(*AUX_MU_IER_REG & 2))
            *AUX_MU_IER_REG |= 2;
    }
    enable_interrupt();
}
void uart_buf_writes_push(char *s){
    // writehex_uart(strlen(s),1);
    // writes_uart(s);
    // *AUX_MU_IER_REG = 1;
    
    for (int i = 0; i < strlen(s); i++)
    {
        /* code */
        if(s[i]=='\n')
            uart_buf_write_push('\r');
        uart_buf_write_push(s[i]);
    }
    
    // while(*s){
    //     if(*s=='\n')
    //         uart_buf_write_push('\r');
    //     uart_buf_write_push(*s++);
    // }
    // *AUX_MU_IER_REG = 3;

}

char uart_buf_read_pop(){
    char c='0';
    if(uart_read_i_l != uart_read_i_r){
        
        c = uart_buf_read[uart_read_i_l++];
        uart_read_i_l=uart_read_i_l%1024;
        
    }
    return c;
}
char uart_buf_write_pop(){
    char c='0';
    if(uart_write_i_l != uart_write_i_r){
        
        c = uart_buf_write[uart_write_i_l++];
        uart_write_i_l= uart_write_i_l % 1024;
        
    }
    return c;
}

int is_empty_write(){
    if(uart_write_i_l==uart_write_i_r){
        return 1; // return 1 if empty.
    }else{
        return 0; // return 0 if not empty.
    }
}
int is_empty_read(){
    if(uart_read_i_l==uart_read_i_r){
        return 1; // return 1 if empty.
    }else{
        return 0; // return 0 if not empty.
    }
}
void busy_wait_writec(char s){
    unsigned int c = s;
    while(!(*AUX_MU_LSR_REG & 0x20)) asm volatile("nop");
    *AUX_MU_IO_REG = c;
}
void busy_wait_writes(char *s,bool newline){
    while(*s){
        if(*s=='\n')
            busy_wait_writec('\r');
        busy_wait_writec(*s++);
    }
    if(newline){
        busy_wait_writec('\r');
        busy_wait_writec('\n');
    }
    
}
void busy_wait_writeint(int s,bool newline){
    if(s==0){
        busy_wait_writec('0');
        if(newline)
            busy_wait_writes("\r\n",FALSE);
        return;
    }
    char a[128];
    int i=0,n=s;
    while(n!=0){
        a[i] = '0' + n%10;
        n/=10;
        i++;
    }
    a[i]='\0';
    for (int j = i-1; j>=0; j--)
    {
        busy_wait_writec(a[j]);
        /* code */
    }
    
    if(newline)
        busy_wait_writes("\r\n",FALSE);
}
void busy_wait_writehex(unsigned long long h,bool newline)
{
    busy_wait_writes("0x",FALSE);
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        
        n=(h>>c)&(0xF); // n = 1,2,3....th byte of h from left to right.
        
        n+=n>9?0x37:0x30; // int 0~9 -> char '0'~'9', 10~15 -> 'A'~'F'
        busy_wait_writec(n);
    }
    if(newline==1){
        busy_wait_writes("\r\n",FALSE);
    }
}