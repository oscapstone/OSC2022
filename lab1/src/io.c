#include "io.h"
#include "uart.h"

void io_init(){
    uart_init();
}

char get_user_inputc(){
    return uart_getc();
}

void putc(char s){
    uart_send(s);
}

void puts(char *s){
    uart_puts(s);
}

void puth(unsigned int d) {
    unsigned int n;
    int c;
    int print = 0;
    uart_puts("0x");
    for(c = 28; c >= 0; c-= 4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        if(n != '0'){
            print = 1;
        }
        if(print){
            uart_send(n);
        }
    }
    if(print == 0){
        uart_send('0');
    }
}
