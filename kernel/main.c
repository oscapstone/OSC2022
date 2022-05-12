#include "dtb.h"

void kernel_main(char* x0) {
    dtb_init(x0);

    task_list_init();
    enable_mini_uart_interrupt();
    enable_interrupt();             // enable interrupt in EL1 -> EL1

    shell();
}