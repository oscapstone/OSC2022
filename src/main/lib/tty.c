#include "tty.h"
#include "uart.h"
#ifndef WITH_STDLIB
#include "type.h"
#else
#include <stdint.h>
#endif


// void tty_recv32b(ttybuf_t* buf) {
//     for(int i=0;i<4;i++) {
//         buf->byte[i] = uart_getc();
//     }
// }

// char tty_recv8b() {
//     return uart_getc();
// }

// void tty_send32b(ttybuf_t* buf) {

//     for(int i=0;i < 4;i++) {
//         uart_putc(buf->byte[i]);
//     }

// }

// void tty_send8b(char c) {
//     uart_putc(c);
// }

void tty_recv(ttybuf_t* buf, size_t s) {

    for(int i=0;i<s;i++) {
        buf->byte[i] = uart_getc();
    }

}

void tty_send(ttybuf_t* buf, size_t s) {
    for(int i=0;i<s;i++)
        uart_putc(buf->byte[i]);
}
