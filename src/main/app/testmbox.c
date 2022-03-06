#include "mailbox.h"
#include "type.h"

#include "uart.h"




void main() {

    init_uart();

    buf[0] = 8 * 4;
    buf[1] = MAILBOX_REQ_CODE;
    buf[2] = 0x00010004;
    buf[3] = 8;
    buf[4] = 8;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = TAG_END;

    mailbox_call(8);
    uart_hex(buf[5]);

    while(1) {
        uart_send(uart_recv());
    }
}