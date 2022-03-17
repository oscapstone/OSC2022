#include "uart.h"
#include "shell.h"

extern char *dtb_place;

void main()
{
    register unsigned long dtb asm("x18");

    dtb_place = dtb;
    uart_init();
    shell();
}
