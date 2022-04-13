#include "uart.h"
#include "string.h"
#include "buddy_system.h"
#include "utils.h"
#include "stdlib.h"

void main() {

    uart_init();
    buddy_system_init();
    mem_init();
    char command[1024];
    unsigned long int ptr[1024];
    int valid[1024];
    for (int i = 0; i < 1024; i++)
        ptr[i] = valid[i] = 0;
    int ptr_num = 0;

    while (1) {
        uart_puts("\r# ");

        get_uart_input(command);

        if (strcmp(command, "A") == 0) {
            uart_puts("size(KB): ");
            get_uart_input(command);
            unsigned long int address = buddy_system_alloc(atoi(command));
            uart_puts("address: 0x");
            uart_hex(address);
            uart_puts("\n");
        } else if (strcmp(command, "D") == 0) {
            uart_puts("index: ");
            get_uart_input(command);
            buddy_system_free(atoi(command));
            uart_puts("\n");
        } else if (strcmp(command, "malloc") == 0) {
            uart_puts("size(B): ");
            get_uart_input(command);
            ptr[ptr_num] = (unsigned long int)malloc(atoi(command));
            uart_puts("index: ");
            uart_hex(ptr_num);
            uart_puts("\n");
            uart_puts("address: 0x");
            uart_hex(ptr[ptr_num]);
            uart_puts("\n");
            valid[ptr_num] = 1;
            ptr_num++;
        } else if (strcmp(command, "free") == 0) {
            uart_puts("index: ");
            get_uart_input(command);
            if (valid[atoi(command)]) {
                free((unsigned long int)ptr[atoi(command)]);
                valid[atoi(command)] = 0;
            } else
                uart_puts("invalid index\n");
        } else
            uart_puts("Error command!\n");
    }
}
