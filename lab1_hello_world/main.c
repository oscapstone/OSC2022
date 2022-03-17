#include "uart.h"
#include "power.h"
#include "string.h"
#include "utils.h"
#include "mbox.h"

int bss_var[4];

int main(void) {
    uart_init();
    uart_puts("Uart Init Done\n");
    // check whether the bss section is set to zero
    uart_num(bss_var[0]); uart_puts("\n");
    uart_num(bss_var[1]); uart_puts("\n");
    uart_num(bss_var[2]); uart_puts("\n");
    uart_num(bss_var[3]); uart_puts("\n");
    uart_puts("=======================\n");
    uart_getc(); // prevent init signal(0xE0) from triggering echo
    
    char commands[256];
    int i = 0;
    char c;

    while(1) {
        c = uart_getc();
        uart_putc(c); // remote echo

        if(c == '\n') { // send command
            uart_putc('\r');
            commands[i] = '\0';
            i = 0;

            if(strcmp(commands, "help")) {
                uart_puts("# help\n"
                          "help\t: print this help menu\n"
                          "hello\t: print Hello World!\n"
                          "mailbox\t: print hardware info\n"
                          "reboot\t: reboot the device\n");
            }
            else if(strcmp(commands, "hello")) {
                uart_puts("Hello World!\n");
            }
            else if(strcmp(commands, "mailbox")) {
                // get the board revision
                mbox_request(MBOX_TAG_GETBOARD_REVISION, 4);
                if (mbox_call(MBOX_CH_PROP)) {
                    uart_puts("Board revision: "); uart_hex(mbox[5]); uart_puts("\n");
                } 
                else {
                    uart_puts("Unable to query serial!\n");
                }

                // get the arm memory base address and size
                mbox_request(MBOX_TAG_GETARM_RAM, 8);
                if (mbox_call(MBOX_CH_PROP)) {
                    uart_puts("Base address: "); uart_hex(mbox[5]); uart_puts("\n");
                    uart_puts("Memory size: ");  uart_num(mbox[6]); uart_puts(" B\n");
                } 
                else {
                    uart_puts("Unable to query serial!\n");
                }
            }
            else if(strcmp(commands, "reboot")) {
                uart_puts("Reboot the board\n");
                delay_ms(1000); // wait for message transmit 
                reset();
            }
            else {
                uart_puts(commands);
                uart_puts(" is not a valid command\n");
            }
        }
        else { // type word
            commands[i++] = c;
        }
    }

    return 0;
}