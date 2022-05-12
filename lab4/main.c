#include "uart.h"
#include "shell.h"
#include "mailbox.h"
#include "irq.h"
#include "time.h"
#include "mm.h"

void main()
{
    timeout_queue_head = 0;
    timeout_queue_tail = 0;
    // set up serial console
    uart_init();
    
    // say hello
    get_board_revision();
    get_arm_memory();
    enable_interrupt();
    // echo everything back
    init_memory_system();
    shell_start();
    
}
