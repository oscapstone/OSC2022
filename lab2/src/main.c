#include "uart.h"
#include "mbox.h"
#include "shell.h"
#include "string.h"
#include "malloc.h"

void main()
{
    //test malloc
    char* test1 = malloc(0x18);
    memcpy(test1,"test malloc1",sizeof("test malloc1"));
    uart_printf("%s\n",test1);
    char* test2 = malloc(0x20);
    memcpy(test2,"test malloc2",sizeof("test malloc2"));
    uart_printf("%s\n",test2);
    char* test3 = malloc(0x28);
    memcpy(test3,"test malloc3",sizeof("test malloc3"));
    uart_printf("%s\n",test3);

    // set up serial console
    uart_init();

    shell();
}
