#include "uart.h"
#include "gpio.h"
#include "type.h"




int main(void) {

    init_uart(270);
    uart_write("Hello world\n");
    while(1) {
        uart_send(uart_recv());
    }
    return 0;

}
