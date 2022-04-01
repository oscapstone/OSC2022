#include "uart.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"
#include "command.h"
extern char __start;
extern char __end;


void command_help ()
{
    uart_puts("help:\tprint this help menu\n");
    uart_puts("hello:\tprint Hello World!\n");
    uart_puts("reboot:\treboot the device\n");
    uart_puts("ls:\treserved space\n");
    uart_puts("cat:\treserved space\n");
    uart_puts("alloac_test:\treserved space\n");
    uart_puts("parse_dtb_header:\treserved space\n");
    uart_puts("\n");
}

void command_hello ()
{
    uart_puts("Hello World!\n");
}


void command_notfound (char * s) 
{
    uart_puts("Err: command ");
    uart_puts(s);
    uart_puts(" not found, try <help>\n");
}

void command_mailbox(){
    get_board_revision();
    get_arm_memory();
}

void command_reboot()
{
    uart_puts("Start Rebooting...\n");

    reset(100);
    
	while(1);
}

void* alloc_addr = (void*)(0x40000);

void alloc_test(){
    char* string = (char*)simple_alloc(40);
    strcpy(string, "test");
    uart_puts(string);
    uart_puts("\n");
}

void* simple_alloc(int size) {
    extern void* alloc_addr;
    alloc_addr = alloc_addr -size;
    return alloc_addr;
}