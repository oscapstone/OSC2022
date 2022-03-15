#include "mini_uart.h"
#include "utils.h"

extern char *_dtb;

void load_kernel() {
    char buffer[MAX_BUFFER_SIZE];

    uart_recv_string(buffer);
    unsigned long k_addr = 0,k_size = 0;
	uart_send_string("Please enter kernel load address (Hex): ");

    uart_recv_string(buffer);
    k_addr = getHexFromString(buffer);
	uart_send_string("Please enter kernel size (Dec): ");
    uart_recv_string(buffer);
    k_size = getIntegerFromString(buffer);

	uart_send_string("Please send kernel image now...\n");
    unsigned char* target=(unsigned char*)k_addr;
    while(k_size--){
        *target=uart_getb();
        target++;
        uart_send('.');
    }
    
    ((void (*)(char *))k_addr)(_dtb);
}

int main() {
    uart_init();
    load_kernel();
    return 0;   // should not reach here
}