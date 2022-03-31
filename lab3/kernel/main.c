#include <stdint.h>

#include "dtb.h"
#include "printf.h"
#include "uart.h"

void kernel_main(char* x0) {
    timer_list_init();
    task_list_init();

    uart_enable_int(RX | TX);
    uart_enable_aux_int();
    dtb_init(x0);
    enable_interrupt();
    shell();
}