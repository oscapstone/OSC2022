#include "uart.h"
#include "mbox.h"
#include "power.h"
#include "string.h"

void main() {
    uart_init();

    while (1) {
        uart_puts("\r# ");
        int i = 0;
        char *command = "";
        char tmp;
        while (1) {
            tmp = uart_getc();
            if (tmp == '\n') {
                uart_puts("\n");
                *(command + i) = '\0';
                break;
            } else
                uart_send(tmp);

            *(command + i) = tmp;
            i++;
        }
        if (strcmp(command, "help") == 0) {
            uart_puts("help\t: print this help memu\n");
            uart_puts("hello\t: print Hello World!\n");
            uart_puts("mailbox\t: print mailbox information\n");
            uart_puts("reboot\t: reboot the device\n");
        } else if (strcmp(command, "hello") == 0)
            uart_puts("Hello World!\n");
        else if (strcmp(command, "mailbox") == 0) {
            if (get_board_revision()) {
                uart_puts("My board revision is : ");
                uart_hex(mbox[5]);
                uart_puts("\n");
            } else
                uart_puts("Failed to get board revision.\n");

            if (get_arm_memory()) {
                uart_puts("My ARM memory base address is : ");
                uart_hex(mbox[5]);
                uart_puts("\n");
                uart_puts("My ARM memory size is : ");
                uart_hex(mbox[6]);
                uart_puts("\n");
            } else
                uart_puts("Failed to get arm memory.\n");

            if (get_serial()) {
                uart_puts("My serial number is: ");
                uart_hex(mbox[6]);
                uart_hex(mbox[5]);
                uart_puts("\n");
            } else
                uart_puts("Failed to get serial.\n");
        } else if (strcmp(command, "reboot") == 0) {
            uart_puts("\n");
            reset();
        } else
            uart_puts("Error command!\n");
    }
}
