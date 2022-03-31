#include "uart.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"
#include "command.h"
#include "time.h"
extern char __start;
extern char __end;


void command_help ()
{
    uart_puts("help:\tprint this help menu\n");
    uart_puts("hello:\tprint Hello World!\n");
    uart_puts("reboot:\treboot the device\n");

    uart_puts("mailbox:\treserved\n");
    uart_puts("reboot:\ttreserved\n");
    uart_puts("kernel_load:\ttreserved\n");
    uart_puts("ls:\treserved\n");
    uart_puts("cat:\ttreserved\n");
    uart_puts("load:\ttreserved\n");
    uart_puts("alloc_test:\ttreserved\n");
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

void kernel_load()
{
    uart_puts("Start copying new kernel to 0x80000\n");
    char* new_kernel_addr = ((char*)0x80000);
    uart_puts("kernel_size:");
    int kernel_size = uart_get_int();
    uart_puts("\n");
    int i;
    for(i=0;i<kernel_size;i++){
        char c = uart_getc();
        uart_send(c);
        *(new_kernel_addr+i) = c;
    }
    void (*new_kernel_start)(void) = (void*)new_kernel_addr;
    new_kernel_start();
}

void* alloc_addr = (void*)(0x70000);

void alloc_test(){
    char* string = (char*)simple_alloc(40);
    strcpy(string, "test");
    uart_puts(string);
    uart_puts("\n");
}

void* simple_alloc(int size) {
    extern void* alloc_addr;
    alloc_addr = alloc_addr-size;
    return (void*)(alloc_addr);
}

void sleep(char *duration){
    if(duration[0]=='\0')
        return;
    int num = 0;
    for(int i=0; duration[i]!='\0'; i++)
        num = num*10 + duration[i] - '0';
    uart_put_int(num);
    char *msg = "test message";
    add_timer(timeout_callback, msg, num);
}

void timeout_callback(char *msg){
    uart_puts("test timeout callback\n");
}