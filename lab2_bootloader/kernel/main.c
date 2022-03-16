#include "uart.h"
#include "power.h"
#include "string.h"
#include "utils.h"
#include "memory.h"
#include "address.h"
#include "fdt.h"
#include "cpio.h"


int main(uint32* addr) {
    uart_init();
    uart_puts("Devicetree: "); uart_hex(addr);
    uart_dem();
    uart_getc(); // flush init signal(0xE0)
    uart_prefix();
    
    char command[256];
    int i = 0;
    char c;

    while(1) {
        c = uart_getc();
        uart_putc(c); // remote echo

        if(c == '\n') { // send command
            uart_putc('\r');
            command[i] = '\0';
            i = 0;

            if(strcmp(command, "help")) {
                uart_puts("# help\n"
                          "help\t\t: print this help menu\n"
                          "ls\t\t: list files\n"
                          "cat [n]\t\t: show file content\n"
                          "allocate\t: allocate memory\n"
                          "devicetree\t: parse devicetree\n"
                          "reboot\t\t: reboot the device");
            }
            else if(strcmp(command, "ls")) {
                extract_cpio(initramfs, 1, 0, 0x00);
            }
            else if(startwith(command, "cat")) {
                 // get the filename
                char* p = command;
                while((*p) != ' ' && (*p++) != '\0');
                p += 1;
                extract_cpio(initramfs, 0, 1, p);
            }
            else if(strcmp(command, "allocate")) {
                uart_puts("Size: ");
                int size = uart_getn();
                char* address = simple_malloc(size);
                uart_puts("Create new space at ");
                uart_hex(address);
                uart_newline();
            }
            else if(strcmp(command, "devicetree")) {
                uart_puts("Start parse: ");
                uart_hex(addr);
                uart_newline();
                fdt_traverse(addr, initramfs_callback);
                uart_puts("Parse Done\n");
            }
            else if(strcmp(command, "reboot")) {
                uart_puts("Reboot the board\n");
                delay_ms(1000); // wait for message transmit 
                reset();
            }
            else {
                uart_puts(command);
                uart_puts(" is not a valid command\n");
            }

            uart_dem();
            uart_prefix();
        }
        else { // type word
            command[i++] = c;
        }
    }

    return 0;
}