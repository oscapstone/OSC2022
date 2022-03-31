#include "mini_uart.h"
#include "reboot.h"
#include "cpio.h"
#include "test.h"
#include "devicetree.h"
 
void input_buffer_overflow_message(char *str){

    uart_printf("input buffer overflow message\n");
}
void command_help(){
    uart_printf("\n");
    uart_printf("# help\n");
    uart_printf("help\t: print this help menu\n");
    uart_printf("hello\t: print Hello World!\n");
    uart_printf("reboot\t: reboot the device\n");
    uart_printf("cancel reboot\t: cancel reboot\n");
    uart_printf("ls\t: list\n");
    uart_printf("cat\t: cat\n");
    uart_printf("test alloc\t: test alloc\n");
    uart_printf("fdt_traverse\t: fdt_traverse\n");
    uart_printf("exec\t: exec FILENAME\n");
    uart_printf("settimeout\t: settimeout MESSAGE SECONDS\n");
    uart_printf("\n");
}

void command_hello(){
    uart_printf("Hello I'm kernel!\n");
}
void command_reboot(){
    uart_printf("rebooting...\n");
    reset();
}

void command_cancel_reboot(){
    uart_printf("cancel reboot\n");
    cancel_reset();
}

void command_not_found(){
    uart_printf("shell: command not found\n");
}

void command_ls(){
    ls();
}

void command_cat(char *str){
    cat(str);
}

void command_test_alloc(){
    test_alloc();
}

void command_fdt_traverse(){
	fdt_traverse();
}

void command_lab3_basic_1(char *str){
	exec(str);
}

void command_settimeout(char *str){
	char *message = strchr(str, ' ') + 1;
	char *end_message = strchr(message, ' ');
	*end_message = '\0';
	char *second = end_message + 1;
	add_timer(uart_send_string, atoi(second) ,message);
}
