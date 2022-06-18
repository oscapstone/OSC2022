#include "uart.h"
#include "string.h"
#include "power.h"
#include "mbox.h"
#include "cpio.h"
#include "timer.h"
#include "exception.h"
#include "alloc.h"
#include "utils.h"
#include "thread.h"
#include "printf.h"
#include "shell.h"
#include "vfs.h"

// #include "device_tree.h"
void main()
{
    // set up serial console
    uart_init();
    buddy_init();
    //welcome message
    uart_puts("*****************************\r\n");
    uart_puts("*       welcome OSC2022     *\r\n");
    uart_puts("*****************************\r\n");
    timeout_event_init();
    enable_interrupt();
    // thread_init();
    vfs_init();
    thread_init();
    run_shell();
}

