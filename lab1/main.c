#include "uart.h"
#include "mbox.h"
#include "shell.h"
#include "lib.h"

void main()
{
    uart_init();
    shell();
}
